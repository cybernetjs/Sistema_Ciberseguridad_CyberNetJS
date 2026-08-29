#include "extractor_caracteristicas.h"

#include <algorithm>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "reloj.h"

namespace sdi {

namespace {

uint16_t leer_entero16_big_endian(const uint8_t* datos) {
    return static_cast<uint16_t>((datos[0] << 8) | datos[1]);
}

constexpr int ENLACE_ETHERNET = 1;
constexpr int ENLACE_LINUX_SLL = 113;
constexpr int ENLACE_LINUX_SLL2 = 276;
constexpr int ENLACE_CRUDO = 12;

struct InformacionCapaEnlace {
    int longitud_cabecera;
    int posicion_ethertype;
    bool tiene_ethertype;
    bool soportado;
};

InformacionCapaEnlace informacion_capa_enlace(int tipo_enlace) {
    switch (tipo_enlace) {
        case ENLACE_ETHERNET:
            return {14, 12, true, true};
        case ENLACE_LINUX_SLL:
            return {16, 14, true, true};
        case ENLACE_LINUX_SLL2:
            return {20, 0, true, true};
        case ENLACE_CRUDO:
            return {0, 0, false, true};
        default:
            return {0, 0, false, false};
    }
}

constexpr uint16_t ETHERTYPE_IP = 0x0800;

}

std::optional<EventoRed> extraer_caracteristicas(const uint8_t* paquete,
                                                   uint32_t longitud_capturada,
                                                   RastreadorFlujos& rastreador,
                                                   int tipo_enlace) {
    InformacionCapaEnlace info = informacion_capa_enlace(tipo_enlace);
    if (!info.soportado) {
        return std::nullopt;
    }

    if (longitud_capturada < static_cast<uint32_t>(info.longitud_cabecera)) {
        return std::nullopt;
    }

    if (info.tiene_ethertype) {
        uint16_t ethertype = leer_entero16_big_endian(paquete + info.posicion_ethertype);
        if (ethertype != ETHERTYPE_IP) {
            return std::nullopt;
        }
    }

    const uint8_t* inicio_ip = paquete + info.longitud_cabecera;
    uint32_t disponible_ip = longitud_capturada - info.longitud_cabecera;
    if (disponible_ip < sizeof(struct ip)) {
        return std::nullopt;
    }

    const auto* cabecera_ip = reinterpret_cast<const struct ip*>(inicio_ip);
    int longitud_cabecera_ip = cabecera_ip->ip_hl * 4;
    if (longitud_cabecera_ip < 20 || static_cast<uint32_t>(longitud_cabecera_ip) > disponible_ip) {
        return std::nullopt;
    }

    char buffer_origen[INET_ADDRSTRLEN] = {0};
    char buffer_destino[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &cabecera_ip->ip_src, buffer_origen, sizeof(buffer_origen));
    inet_ntop(AF_INET, &cabecera_ip->ip_dst, buffer_destino, sizeof(buffer_destino));

    if (buffer_origen[0] == '\0' || buffer_destino[0] == '\0') {
        return std::nullopt;
    }

    int protocolo = cabecera_ip->ip_p;

    const uint8_t* inicio_capa4 = inicio_ip + longitud_cabecera_ip;
    uint32_t disponible_capa4 = disponible_ip - longitud_cabecera_ip;

    int puerto_origen = 1;
    int puerto_destino = 1;
    if ((protocolo == IPPROTO_TCP || protocolo == IPPROTO_UDP) && disponible_capa4 >= 4) {
        puerto_origen = leer_entero16_big_endian(inicio_capa4);
        puerto_destino = leer_entero16_big_endian(inicio_capa4 + 2);
    }

    double instante_actual = tiempo::segundos_actuales();
    int longitud_paquete = static_cast<int>(longitud_capturada);

    ResultadoFlujo resultado_flujo = rastreador.actualizar_flujo(
        buffer_origen, puerto_origen, buffer_destino, puerto_destino,
        protocolo, longitud_paquete, instante_actual);

    int ttl_origen = cabecera_ip->ip_ttl;
    double duracion = resultado_flujo.duracion;

    EventoRed evento;
    evento.ip_origen = buffer_origen;
    evento.ip_destino = buffer_destino;
    evento.puerto_origen = puerto_origen;
    evento.puerto_destino = puerto_destino;
    evento.protocolo = protocolo;
    evento.duracion = duracion;
    evento.paquetes_origen = 1;
    evento.paquetes_destino = 0;
    evento.bytes_origen = longitud_paquete;
    evento.bytes_destino = 0;
    evento.tasa_transferencia = 1.0 / duracion;
    evento.ttl_origen = ttl_origen;
    evento.ttl_destino = 0;
    evento.carga_origen = longitud_paquete / duracion;
    evento.carga_destino = 0.0;
    evento.intervalo_origen = duracion;
    evento.intervalo_destino = duracion;
    evento.fluctuacion_origen = 0.0;
    evento.fluctuacion_destino = 0.0;
    evento.conteo_servicio_origen = 1;
    evento.conteo_destino_reciente = 1;
    evento.orig_pkts_flujo = resultado_flujo.orig_pkts;
    evento.orig_ip_bytes_flujo = resultado_flujo.orig_ip_bytes;
    evento.resp_pkts_flujo = resultado_flujo.resp_pkts;
    evento.resp_ip_bytes_flujo = resultado_flujo.resp_ip_bytes;
    evento.missed_bytes = 0;

    return evento;
}

}