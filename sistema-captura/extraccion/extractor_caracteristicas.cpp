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

constexpr uint8_t BANDERA_TCP_FIN = 0x01;
constexpr uint8_t BANDERA_TCP_SYN = 0x02;
constexpr uint8_t BANDERA_TCP_ACK = 0x10;

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

std::string analizar_consulta_dns(const uint8_t* base, uint32_t disponible) {
    if (disponible < 13) {
        return "";
    }

    const uint8_t* cursor = base + 12;
    uint32_t restante = disponible - 12;
    std::string dominio;
    int etiquetas = 0;

    while (restante > 0 && etiquetas < 30) {
        uint8_t longitud = cursor[0];
        if (longitud == 0) {
            break;
        }
        if ((longitud & 0xC0) == 0xC0) {
            return "";
        }
        if (restante < static_cast<uint32_t>(longitud) + 1) {
            return "";
        }
        if (!dominio.empty()) {
            dominio += ".";
        }
        for (uint8_t i = 0; i < longitud; i++) {
            char c = static_cast<char>(cursor[1 + i]);
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-') {
                return "";
            }
            dominio += c;
        }
        cursor += longitud + 1;
        restante -= longitud + 1;
        etiquetas++;
    }

    return dominio;
}

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
    bool es_syn = false;

    if ((protocolo == IPPROTO_TCP || protocolo == IPPROTO_UDP) && disponible_capa4 >= 4) {
        puerto_origen = leer_entero16_big_endian(inicio_capa4);
        puerto_destino = leer_entero16_big_endian(inicio_capa4 + 2);
    }

    if (protocolo == IPPROTO_TCP && disponible_capa4 >= 14) {
        uint8_t banderas = inicio_capa4[13];
        es_syn = (banderas & BANDERA_TCP_SYN) != 0 && (banderas & BANDERA_TCP_ACK) == 0;
    }

    std::string consulta_dns;
    if (protocolo == IPPROTO_UDP && puerto_destino == 53 && disponible_capa4 >= 8) {
        const uint8_t* inicio_udp_payload = inicio_capa4 + 8;
        uint32_t disponible_udp_payload = disponible_capa4 - 8;
        consulta_dns = analizar_consulta_dns(inicio_udp_payload, disponible_udp_payload);
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
    evento.consulta_dns = consulta_dns;
    evento.es_syn = es_syn;

    return evento;
}

}