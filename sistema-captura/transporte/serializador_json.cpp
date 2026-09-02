#include "serializador_json.h"

#include <nlohmann/json.hpp>

namespace sdi {

using json = nlohmann::json;

namespace {

json evento_a_objeto_json(const EventoRed& evento) {
    return json{
        {"ip_origen", evento.ip_origen},
        {"ip_destino", evento.ip_destino},
        {"puerto_origen", evento.puerto_origen},
        {"puerto_destino", evento.puerto_destino},
        {"protocolo", evento.protocolo},
        {"duracion", evento.duracion},
        {"paquetes_origen", evento.paquetes_origen},
        {"paquetes_destino", evento.paquetes_destino},
        {"bytes_origen", evento.bytes_origen},
        {"bytes_destino", evento.bytes_destino},
        {"tasa_transferencia", evento.tasa_transferencia},
        {"ttl_origen", evento.ttl_origen},
        {"ttl_destino", evento.ttl_destino},
        {"carga_origen", evento.carga_origen},
        {"carga_destino", evento.carga_destino},
        {"intervalo_origen", evento.intervalo_origen},
        {"intervalo_destino", evento.intervalo_destino},
        {"fluctuacion_origen", evento.fluctuacion_origen},
        {"fluctuacion_destino", evento.fluctuacion_destino},
        {"conteo_servicio_origen", evento.conteo_servicio_origen},
        {"conteo_destino_reciente", evento.conteo_destino_reciente},
        {"orig_pkts_flujo", evento.orig_pkts_flujo},
        {"orig_ip_bytes_flujo", evento.orig_ip_bytes_flujo},
        {"resp_pkts_flujo", evento.resp_pkts_flujo},
        {"resp_ip_bytes_flujo", evento.resp_ip_bytes_flujo},
        {"missed_bytes", evento.missed_bytes},
        {"consulta_dns", evento.consulta_dns},
        {"es_syn", evento.es_syn},
    };
}

}

std::string evento_a_json(const EventoRed& evento) {
    return evento_a_objeto_json(evento).dump();
}

std::string lote_a_json(const std::vector<EventoRed>& lote) {
    if (lote.size() == 1) {
        return evento_a_objeto_json(lote.front()).dump();
    }
    json arreglo = json::array();
    arreglo.get_ref<json::array_t&>().reserve(lote.size());
    for (const auto& evento : lote) {
        arreglo.push_back(evento_a_objeto_json(evento));
    }
    return arreglo.dump();
}

}