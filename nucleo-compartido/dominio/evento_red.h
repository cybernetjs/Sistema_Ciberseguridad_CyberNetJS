#pragma once

#include <string>

namespace sdi {

struct EventoRed {
    std::string ip_origen;
    std::string ip_destino;
    int puerto_origen = 1;
    int puerto_destino = 1;
    int protocolo = 0;
    double duracion = 0.001;
    int paquetes_origen = 1;
    int paquetes_destino = 0;
    int bytes_origen = 0;
    int bytes_destino = 0;
    double tasa_transferencia = 0.0;
    int ttl_origen = 64;
    int ttl_destino = 0;
    double carga_origen = 0.0;
    double carga_destino = 0.0;
    double intervalo_origen = 0.0;
    double intervalo_destino = 0.0;
    double fluctuacion_origen = 0.0;
    double fluctuacion_destino = 0.0;
    int conteo_servicio_origen = 1;
    int conteo_destino_reciente = 1;
    long orig_pkts_flujo = 0;
    long orig_ip_bytes_flujo = 0;
    long resp_pkts_flujo = 0;
    long resp_ip_bytes_flujo = 0;
    int missed_bytes = 0;
    std::string consulta_dns;
    bool es_syn = false;
};

}