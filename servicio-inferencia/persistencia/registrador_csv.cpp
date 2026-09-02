#include "registrador_csv.h"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>

#include "bitacora.h"

namespace sdi {

namespace {

std::string campo_csv(const std::string& valor) {
    bool necesita_comillas = valor.find(',') != std::string::npos ||
                              valor.find('"') != std::string::npos ||
                              valor.find('\n') != std::string::npos;
    if (!necesita_comillas) {
        return valor;
    }
    std::string escapado = valor;
    std::string resultado = "\"";
    for (char caracter : escapado) {
        if (caracter == '"') {
            resultado += "\"\"";
        } else {
            resultado += caracter;
        }
    }
    resultado += "\"";
    return resultado;
}

}

RegistradorCsv::RegistradorCsv(const std::string& ruta_archivo) : ruta_archivo_(ruta_archivo) {
    bool ya_existe = std::filesystem::exists(ruta_archivo_) && std::filesystem::file_size(ruta_archivo_) > 0;

    archivo_.open(ruta_archivo_, std::ios::app);
    if (!archivo_.is_open()) {
        Bitacora::instancia().registrar_error("No se pudo abrir el archivo de registro CSV: " + ruta_archivo_);
        return;
    }

    if (!ya_existe) {
        escribir_encabezado();
    }

    Bitacora::instancia().registrar_info("Registrando eventos procesados en: " + ruta_archivo_);
}

bool RegistradorCsv::listo() const { return const_cast<std::ofstream&>(archivo_).is_open(); }

void RegistradorCsv::escribir_encabezado() {
    archivo_ << "marca_tiempo_unix,ip_origen,ip_destino,puerto_origen,puerto_destino,protocolo,"
                "duracion,paquetes_origen,paquetes_destino,bytes_origen,bytes_destino,"
                "tasa_transferencia,ttl_origen,ttl_destino,carga_origen,carga_destino,"
                "intervalo_origen,intervalo_destino,fluctuacion_origen,fluctuacion_destino,"
                "conteo_servicio_origen,conteo_destino_reciente,"
                "orig_pkts_flujo,orig_ip_bytes_flujo,resp_pkts_flujo,resp_ip_bytes_flujo,missed_bytes,"
                "clasificador,es_amenaza,etiqueta,confianza,tiempo_respuesta_ms\n";
    archivo_.flush();
}

void RegistradorCsv::registrar(const EventoRed& evento, const VeredictoClasificacion& veredicto,
                                const std::string& clasificador, double tiempo_respuesta_ms) {
    std::lock_guard<std::mutex> bloqueo(mutex_);
    if (!archivo_.is_open()) {
        return;
    }

    auto marca_tiempo = std::chrono::system_clock::now().time_since_epoch();
    double marca_tiempo_unix = std::chrono::duration<double>(marca_tiempo).count();

    std::ostringstream fila;
    fila << std::fixed << std::setprecision(6);
    fila << marca_tiempo_unix << ','
         << campo_csv(evento.ip_origen) << ','
         << campo_csv(evento.ip_destino) << ','
         << evento.puerto_origen << ','
         << evento.puerto_destino << ','
         << evento.protocolo << ','
         << evento.duracion << ','
         << evento.paquetes_origen << ','
         << evento.paquetes_destino << ','
         << evento.bytes_origen << ','
         << evento.bytes_destino << ','
         << evento.tasa_transferencia << ','
         << evento.ttl_origen << ','
         << evento.ttl_destino << ','
         << evento.carga_origen << ','
         << evento.carga_destino << ','
         << evento.intervalo_origen << ','
         << evento.intervalo_destino << ','
         << evento.fluctuacion_origen << ','
         << evento.fluctuacion_destino << ','
         << evento.conteo_servicio_origen << ','
         << evento.conteo_destino_reciente << ','
         << evento.orig_pkts_flujo << ','
         << evento.orig_ip_bytes_flujo << ','
         << evento.resp_pkts_flujo << ','
         << evento.resp_ip_bytes_flujo << ','
         << evento.missed_bytes << ','
         << campo_csv(clasificador) << ','
         << (veredicto.es_amenaza ? 1 : 0) << ','
         << campo_csv(veredicto.etiqueta) << ','
         << veredicto.confianza << ','
         << tiempo_respuesta_ms << '\n';

    archivo_ << fila.str();
    archivo_.flush();
}

}