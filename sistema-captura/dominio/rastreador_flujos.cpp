#include "rastreador_flujos.h"

#include <algorithm>

#include "reloj.h"

namespace sdi {

namespace {

ClaveFlujo construir_clave(const std::string& ip1, int puerto1, const std::string& ip2, int puerto2, int protocolo) {
    if (ip1 < ip2 || (ip1 == ip2 && puerto1 <= puerto2)) {
        return ClaveFlujo{ip1, ip2, puerto1, puerto2, protocolo};
    }
    return ClaveFlujo{ip2, ip1, puerto2, puerto1, protocolo};
}

}

RastreadorFlujos::RastreadorFlujos(double edad_maxima_segundos, std::chrono::seconds intervalo_limpieza,
                                    double ventana_maxima_segundos)
    : inicio_(tiempo::segundos_actuales()),
      edad_maxima_segundos_(edad_maxima_segundos),
      ventana_maxima_segundos_(ventana_maxima_segundos),
      intervalo_limpieza_(intervalo_limpieza) {
    hilo_limpieza_ = std::thread(&RastreadorFlujos::ciclo_limpieza, this);
}

RastreadorFlujos::~RastreadorFlujos() {
    {
        std::lock_guard<std::mutex> bloqueo(mutex_variable_condicion_);
        en_ejecucion_ = false;
    }
    variable_condicion_.notify_all();
    if (hilo_limpieza_.joinable()) {
        hilo_limpieza_.join();
    }
}

ResultadoFlujo RastreadorFlujos::actualizar_flujo(const std::string& ip_origen, int puerto_origen,
                                                   const std::string& ip_destino, int puerto_destino,
                                                   int protocolo, int longitud_paquete, double instante_actual) {
    ClaveFlujo clave = construir_clave(ip_origen, puerto_origen, ip_destino, puerto_destino, protocolo);

    std::lock_guard<std::mutex> bloqueo(mutex_mapa_);
    auto it = flujos_.find(clave);
    if (it == flujos_.end()) {
        EstadoFlujo estado;
        estado.primera_actividad = instante_actual;
        estado.ultima_actividad = instante_actual;
        estado.ip_iniciador = ip_origen;
        estado.puerto_iniciador = puerto_origen;
        it = flujos_.emplace(clave, estado).first;
    }

    EstadoFlujo& estado = it->second;

    if (instante_actual - estado.primera_actividad > ventana_maxima_segundos_) {
        estado.primera_actividad = instante_actual;
        estado.ip_iniciador = ip_origen;
        estado.puerto_iniciador = puerto_origen;
        estado.orig_pkts = 0;
        estado.orig_ip_bytes = 0;
        estado.resp_pkts = 0;
        estado.resp_ip_bytes = 0;
    }

    bool es_iniciador = (ip_origen == estado.ip_iniciador && puerto_origen == estado.puerto_iniciador);

    if (es_iniciador) {
        estado.orig_pkts += 1;
        estado.orig_ip_bytes += longitud_paquete;
    } else {
        estado.resp_pkts += 1;
        estado.resp_ip_bytes += longitud_paquete;
    }

    estado.ultima_actividad = instante_actual;

    ResultadoFlujo resultado;
    resultado.duracion = std::max(0.001, estado.ultima_actividad - estado.primera_actividad);
    resultado.orig_pkts = estado.orig_pkts;
    resultado.orig_ip_bytes = estado.orig_ip_bytes;
    resultado.resp_pkts = estado.resp_pkts;
    resultado.resp_ip_bytes = estado.resp_ip_bytes;
    return resultado;
}

size_t RastreadorFlujos::flujos_rastreados() const {
    std::lock_guard<std::mutex> bloqueo(mutex_mapa_);
    return flujos_.size();
}

void RastreadorFlujos::limpiar_una_vez(double instante_actual) {
    std::lock_guard<std::mutex> bloqueo(mutex_mapa_);
    for (auto it = flujos_.begin(); it != flujos_.end();) {
        if (instante_actual - it->second.ultima_actividad > edad_maxima_segundos_) {
            it = flujos_.erase(it);
        } else {
            ++it;
        }
    }
}

void RastreadorFlujos::ciclo_limpieza() {
    std::unique_lock<std::mutex> bloqueo(mutex_variable_condicion_);
    while (en_ejecucion_) {
        bool detenido = variable_condicion_.wait_for(bloqueo, intervalo_limpieza_,
                                                       [this] { return !en_ejecucion_.load(); });
        if (detenido) {
            break;
        }
        limpiar_una_vez(tiempo::segundos_actuales());
    }
}

}