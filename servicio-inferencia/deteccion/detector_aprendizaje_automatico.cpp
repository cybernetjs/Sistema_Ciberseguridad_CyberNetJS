#include "detector_aprendizaje_automatico.h"

#include <algorithm>
#include <cmath>
#include <fstream>

#include "nlohmann/json.hpp"
#include "reloj.h"

namespace sdi {

namespace {

constexpr int PROTOCOLO_TCP = 6;
constexpr int PROTOCOLO_UDP = 17;

constexpr double CAP_ORIG_IP_BYTES = 2500000.0;
constexpr double CAP_RESP_IP_BYTES = 2500000.0;
constexpr double CAP_ORIG_PKTS = 15000.0;
constexpr double CAP_RESP_PKTS = 15000.0;
constexpr double CAP_DURACION_SEGUNDOS = 300.0;

void parsear_nodo(const nlohmann::json& nodo_json, ArbolXgboost& arbol) {
    NodoArbol nodo;
    int id = nodo_json.at("nodeid").get<int>();

    if (nodo_json.contains("leaf")) {
        nodo.es_hoja = true;
        nodo.valor_hoja = nodo_json.at("leaf").get<double>();
    } else {
        nodo.es_hoja = false;
        std::string split = nodo_json.at("split").get<std::string>();
        nodo.indice_caracteristica = std::stoi(split.substr(1));
        nodo.condicion = nodo_json.at("split_condition").get<double>();
        nodo.id_si = nodo_json.at("yes").get<int>();
        nodo.id_no = nodo_json.at("no").get<int>();
    }

    arbol[id] = nodo;

    if (nodo_json.contains("children")) {
        for (const auto& hijo : nodo_json.at("children")) {
            parsear_nodo(hijo, arbol);
        }
    }
}

}

bool DetectorAprendizajeAutomatico::cargar_modelo(const std::string& ruta_modelo) {
    ruta_modelo_ = ruta_modelo;
    modelo_cargado_ = false;

    std::ifstream archivo(ruta_modelo_);
    if (!archivo.is_open()) {
        return false;
    }

    nlohmann::json contenido;
    archivo >> contenido;

    orden_caracteristicas_ = contenido.at("orden_caracteristicas").get<std::vector<std::string>>();
    media_ = contenido.at("media").get<std::vector<double>>();
    desviacion_ = contenido.at("desviacion").get<std::vector<double>>();
    sesgo_inicial_ = contenido.at("sesgo_inicial").get<std::vector<double>>();
    num_clases_ = contenido.value("num_clases", 1);
    nombres_clases_ = contenido.value("clases", std::vector<std::string>{"benigno", "ataque"});

    arboles_.clear();
    for (const auto& arbol_json : contenido.at("arboles")) {
        ArbolXgboost arbol;
        parsear_nodo(arbol_json, arbol);
        arboles_.push_back(std::move(arbol));
    }

    modelo_cargado_ = true;
    return true;
}

std::vector<double> DetectorAprendizajeAutomatico::construir_vector_caracteristicas(const EventoRed& evento) const {
    std::vector<double> crudo;
    crudo.reserve(orden_caracteristicas_.size());

    double duracion_segura = evento.duracion > 0.001 ? evento.duracion : 0.001;
    duracion_segura = std::min(duracion_segura, CAP_DURACION_SEGUNDOS);

    double orig_pkts_recortado = std::min(static_cast<double>(evento.orig_pkts_flujo), CAP_ORIG_PKTS);
    double resp_pkts_recortado = std::min(static_cast<double>(evento.resp_pkts_flujo), CAP_RESP_PKTS);
    double orig_ip_bytes_recortado = std::min(static_cast<double>(evento.orig_ip_bytes_flujo), CAP_ORIG_IP_BYTES);
    double resp_ip_bytes_recortado = std::min(static_cast<double>(evento.resp_ip_bytes_flujo), CAP_RESP_IP_BYTES);

    for (const std::string& nombre : orden_caracteristicas_) {
        double valor = 0.0;
        if (nombre == "id.orig_p") {
            valor = static_cast<double>(evento.puerto_origen);
        } else if (nombre == "id.resp_p") {
            valor = static_cast<double>(evento.puerto_destino);
        } else if (nombre == "missed_bytes") {
            valor = static_cast<double>(evento.missed_bytes);
        } else if (nombre == "orig_pkts") {
            valor = orig_pkts_recortado;
        } else if (nombre == "orig_ip_bytes") {
            valor = orig_ip_bytes_recortado;
        } else if (nombre == "resp_pkts") {
            valor = resp_pkts_recortado;
        } else if (nombre == "resp_ip_bytes") {
            valor = resp_ip_bytes_recortado;
        } else if (nombre == "duration") {
            valor = duracion_segura;
        } else if (nombre == "orig_bytes") {
            valor = orig_ip_bytes_recortado;
        } else if (nombre == "resp_bytes") {
            valor = resp_ip_bytes_recortado;
        } else if (nombre == "proto") {
            valor = static_cast<double>(evento.protocolo);
        } else if (nombre == "conexiones_origen_5s") {
            valor = static_cast<double>(evento.conexiones_origen_5s);
        } else if (nombre == "puertos_distintos_origen_5s") {
            valor = static_cast<double>(evento.puertos_distintos_origen_5s);
        } else if (nombre == "ips_distintas_origen_60s") {
            valor = static_cast<double>(evento.ips_distintas_origen_60s);
        } else if (nombre == "conexiones_mismo_destino_300s") {
            valor = static_cast<double>(evento.conexiones_mismo_destino_300s);
        }
        crudo.push_back(valor);
    }

    std::vector<double> normalizado(crudo.size());
    for (size_t i = 0; i < crudo.size(); i++) {
        double desv = desviacion_[i] == 0.0 ? 1.0 : desviacion_[i];
        normalizado[i] = (crudo[i] - media_[i]) / desv;
    }

    return normalizado;
}

double DetectorAprendizajeAutomatico::evaluar_arbol(const ArbolXgboost& arbol, const std::vector<double>& caracteristicas) const {
    int id_actual = 0;
    while (true) {
        const NodoArbol& nodo = arbol.at(id_actual);
        if (nodo.es_hoja) {
            return nodo.valor_hoja;
        }
        double valor = caracteristicas[nodo.indice_caracteristica];
        id_actual = (valor < nodo.condicion) ? nodo.id_si : nodo.id_no;
    }
}

std::string DetectorAprendizajeAutomatico::construir_clave_flujo(const EventoRed& evento) const {
    std::string ip_a = evento.ip_origen;
    std::string ip_b = evento.ip_destino;
    int puerto_a = evento.puerto_origen;
    int puerto_b = evento.puerto_destino;

    if (ip_a < ip_b || (ip_a == ip_b && puerto_a <= puerto_b)) {
        return ip_a + ":" + std::to_string(puerto_a) + "-" + ip_b + ":" + std::to_string(puerto_b) + "-" +
               std::to_string(evento.protocolo);
    }
    return ip_b + ":" + std::to_string(puerto_b) + "-" + ip_a + ":" + std::to_string(puerto_a) + "-" +
           std::to_string(evento.protocolo);
}

bool DetectorAprendizajeAutomatico::clase_requiere_gate_volumen(const std::string& clase) const {
    return clase == "ddos" || clase == "dos";
}

double DetectorAprendizajeAutomatico::umbral_aplicable_para_clase(const std::string& clase) const {
    return clase_requiere_gate_volumen(clase) ? umbral_probabilidad_alerta_volumen_ : umbral_probabilidad_alerta_;
}

DetectorAprendizajeAutomatico::ResultadoModelo DetectorAprendizajeAutomatico::evaluar_modelo(const EventoRed& evento) const {
    ResultadoModelo resultado;

    std::vector<double> caracteristicas = construir_vector_caracteristicas(evento);

    std::vector<double> margenes(num_clases_, 0.0);
    for (size_t i = 0; i < static_cast<size_t>(num_clases_); i++) {
        margenes[i] = sesgo_inicial_[i];
    }

    for (size_t i = 0; i < arboles_.size(); i++) {
        int clase = static_cast<int>(i % static_cast<size_t>(num_clases_));
        margenes[clase] += evaluar_arbol(arboles_[i], caracteristicas);
    }

    if (num_clases_ <= 1) {
        double probabilidad_ataque = 1.0 / (1.0 + std::exp(-margenes[0]));
        if (probabilidad_ataque > 0.5) {
            resultado.clase_predicha = "ataque";
            resultado.probabilidad_clase_predicha = probabilidad_ataque;
        } else {
            resultado.clase_predicha = "benigno";
            resultado.probabilidad_clase_predicha = 1.0 - probabilidad_ataque;
        }
        return resultado;
    }

    double maximo = *std::max_element(margenes.begin(), margenes.end());
    std::vector<double> exponenciales(num_clases_);
    double suma = 0.0;
    for (int i = 0; i < num_clases_; i++) {
        exponenciales[i] = std::exp(margenes[i] - maximo);
        suma += exponenciales[i];
    }

    int indice_maximo = 0;
    double probabilidad_maxima = 0.0;
    for (int i = 0; i < num_clases_; i++) {
        double probabilidad = exponenciales[i] / suma;
        if (probabilidad > probabilidad_maxima) {
            probabilidad_maxima = probabilidad;
            indice_maximo = i;
        }
    }

    resultado.clase_predicha = indice_maximo < static_cast<int>(nombres_clases_.size())
                                     ? nombres_clases_[indice_maximo]
                                     : ("clase_" + std::to_string(indice_maximo));
    resultado.probabilidad_clase_predicha = probabilidad_maxima;
    return resultado;
}

VeredictoClasificacion DetectorAprendizajeAutomatico::clasificar(const EventoRed& evento) {
    VeredictoClasificacion veredicto;

    if (!modelo_cargado_) {
        return veredicto;
    }

    if (evento.protocolo != PROTOCOLO_TCP && evento.protocolo != PROTOCOLO_UDP) {
        return veredicto;
    }

    ResultadoModelo resultado = evaluar_modelo(evento);

    if (resultado.clase_predicha == "benigno") {
        return veredicto;
    }

    long total_paquetes_flujo = evento.orig_pkts_flujo + evento.resp_pkts_flujo;
    double duracion_segura = evento.duracion > 0.001 ? evento.duracion : 0.001;
    double pps_flujo = static_cast<double>(total_paquetes_flujo) / duracion_segura;

    bool supera_probabilidad = resultado.probabilidad_clase_predicha > umbral_aplicable_para_clase(resultado.clase_predicha);

    bool cumple_volumen = true;
    if (clase_requiere_gate_volumen(resultado.clase_predicha)) {
        bool supera_volumen = total_paquetes_flujo >= paquetes_minimos_alerta_;
        bool supera_tasa = pps_flujo >= pps_minimo_alerta_;
        cumple_volumen = supera_volumen && supera_tasa;
    }

    if (!(supera_probabilidad && cumple_volumen)) {
        return veredicto;
    }

    std::string clave_flujo = construir_clave_flujo(evento);
    double ahora = tiempo::segundos_actuales();

    auto it = ultima_alerta_por_flujo_.find(clave_flujo);
    if (it != ultima_alerta_por_flujo_.end() && (ahora - it->second) < cooldown_alerta_segundos_) {
        return veredicto;
    }

    ultima_alerta_por_flujo_[clave_flujo] = ahora;

    veredicto.es_amenaza = true;
    veredicto.etiqueta = resultado.clase_predicha;
    veredicto.confianza = resultado.probabilidad_clase_predicha;

    return veredicto;
}

DiagnosticoIA DetectorAprendizajeAutomatico::diagnosticar(const EventoRed& evento) const {
    DiagnosticoIA diagnostico;

    if (!modelo_cargado_) {
        return diagnostico;
    }

    if (evento.protocolo != PROTOCOLO_TCP && evento.protocolo != PROTOCOLO_UDP) {
        return diagnostico;
    }

    ResultadoModelo resultado = evaluar_modelo(evento);

    long total_paquetes_flujo = evento.orig_pkts_flujo + evento.resp_pkts_flujo;
    double duracion_segura = evento.duracion > 0.001 ? evento.duracion : 0.001;
    double pps_flujo = static_cast<double>(total_paquetes_flujo) / duracion_segura;

    bool supera_probabilidad = resultado.probabilidad_clase_predicha > umbral_aplicable_para_clase(resultado.clase_predicha);

    bool cumple_volumen = true;
    if (clase_requiere_gate_volumen(resultado.clase_predicha)) {
        bool supera_volumen = total_paquetes_flujo >= paquetes_minimos_alerta_;
        bool supera_tasa = pps_flujo >= pps_minimo_alerta_;
        cumple_volumen = supera_volumen && supera_tasa;
    }

    diagnostico.evaluado = true;
    diagnostico.probabilidad = resultado.probabilidad_clase_predicha;
    diagnostico.tipo_predicho = resultado.clase_predicha;
    diagnostico.es_amenaza = resultado.clase_predicha != "benigno" && supera_probabilidad && cumple_volumen;

    return diagnostico;
}

std::string DetectorAprendizajeAutomatico::nombre() const { return "aprendizaje_automatico"; }

}