#include "decodificador_eventos_json.h"

#include <nlohmann/json.hpp>

namespace sdi {

using json = nlohmann::json;

namespace {

EventoRed decodificar_uno(const json& objeto) {
    EventoRed evento;
    evento.ip_origen = objeto.value("ip_origen", std::string());
    evento.ip_destino = objeto.value("ip_destino", std::string());
    evento.puerto_destino = objeto.value("puerto_destino", 1);
    evento.protocolo = objeto.value("protocolo", 0);
    evento.duracion = objeto.value("duracion", 0.001);
    evento.paquetes_origen = objeto.value("paquetes_origen", 1);
    evento.paquetes_destino = objeto.value("paquetes_destino", 0);
    evento.bytes_origen = objeto.value("bytes_origen", 0);
    evento.bytes_destino = objeto.value("bytes_destino", 0);
    evento.tasa_transferencia = objeto.value("tasa_transferencia", 0.0);
    evento.ttl_origen = objeto.value("ttl_origen", 64);
    evento.ttl_destino = objeto.value("ttl_destino", 0);
    evento.carga_origen = objeto.value("carga_origen", 0.0);
    evento.carga_destino = objeto.value("carga_destino", 0.0);
    evento.intervalo_origen = objeto.value("intervalo_origen", 0.0);
    evento.intervalo_destino = objeto.value("intervalo_destino", 0.0);
    evento.fluctuacion_origen = objeto.value("fluctuacion_origen", 0.0);
    evento.fluctuacion_destino = objeto.value("fluctuacion_destino", 0.0);
    evento.conteo_servicio_origen = objeto.value("conteo_servicio_origen", 1);
    evento.conteo_destino_reciente = objeto.value("conteo_destino_reciente", 1);
    return evento;
}

}

std::vector<EventoRed> decodificar_eventos(const std::string& linea) {
    std::vector<EventoRed> eventos;

    json contenido;
    try {
        contenido = json::parse(linea);
    } catch (const json::parse_error&) {
        return eventos;
    }

    if (contenido.is_array()) {
        eventos.reserve(contenido.size());
        for (const auto& elemento : contenido) {
            if (elemento.is_object()) {
                eventos.push_back(decodificar_uno(elemento));
            }
        }
    } else if (contenido.is_object()) {
        eventos.push_back(decodificar_uno(contenido));
    }

    return eventos;
}

}
