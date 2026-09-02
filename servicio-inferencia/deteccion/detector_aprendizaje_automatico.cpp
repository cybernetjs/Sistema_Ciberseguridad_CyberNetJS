#include "detector_aprendizaje_automatico.h"

#include <cmath>
#include <fstream>

#include "nlohmann/json.hpp"
#include "reloj.h"

namespace sdi {

namespace {

constexpr int PROTOCOLO_TCP = 6;
constexpr int PROTOCOLO_UDP = 17;

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
    sesgo_inicial_ = contenido.at("sesgo_inicial").get<double>();

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

    for (const std::string& nombre : orden_caracteristicas_) {
        double valor = 0.0;
        if (nombre == "id.orig_p") {
            valor = static_cast<double>(evento.puerto_origen);
        } else if (nombre == "id.resp_p") {
            valor = static_cast<double>(evento.puerto_destino);
        } else if (nombre == "missed_bytes") {
            valor = static_cast<double>(evento.missed_bytes);
        } else if (nombre == "orig_pkts") {
            valor = static_cast<double>(evento.orig_pkts_flujo);
        } else if (nombre == "orig_ip_bytes") {
            valor = static_cast<double>(evento.orig_ip_bytes_flujo);
        } else if (nombre == "resp_pkts") {
            valor = static_cast<double>(evento.resp_pkts_flujo);
        } else if (nombre == "resp_ip_bytes") {
            valor = static_cast<double>(evento.resp_ip_bytes_flujo);
        } else if (nombre == "duration") {
            valor = duracion_segura;
        } else if (nombre == "orig_bytes") {
            valor = static_cast<double>(evento.orig_ip_bytes_flujo);
        } else if (nombre == "resp_bytes") {
            valor = static_cast<double>(evento.resp_ip_bytes_flujo);
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

VeredictoClasificacion DetectorAprendizajeAutomatico::clasificar(const EventoRed& evento) {
    VeredictoClasificacion veredicto;

    if (!modelo_cargado_) {
        return veredicto;
    }

    if (evento.protocolo != PROTOCOLO_TCP && evento.protocolo != PROTOCOLO_UDP) {
        return veredicto;
    }

    std::vector<double> caracteristicas = construir_vector_caracteristicas(evento);

    double margen = sesgo_inicial_;
    for (const auto& arbol : arboles_) {
        margen += evaluar_arbol(arbol, caracteristicas);
    }

    double probabilidad = 1.0 / (1.0 + std::exp(-margen));
    long total_paquetes_flujo = evento.orig_pkts_flujo + evento.resp_pkts_flujo;
    double duracion_segura = evento.duracion > 0.001 ? evento.duracion : 0.001;
    double pps_flujo = static_cast<double>(total_paquetes_flujo) / duracion_segura;

    bool supera_probabilidad = probabilidad > umbral_probabilidad_alerta_;
    bool supera_volumen = total_paquetes_flujo >= paquetes_minimos_alerta_;
    bool supera_tasa = pps_flujo >= pps_minimo_alerta_;

    if (!(supera_probabilidad && supera_volumen && supera_tasa)) {
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
    veredicto.etiqueta = "ataque";
    veredicto.confianza = probabilidad;

    return veredicto;
}

std::string DetectorAprendizajeAutomatico::nombre() const { return "aprendizaje_automatico"; }

}