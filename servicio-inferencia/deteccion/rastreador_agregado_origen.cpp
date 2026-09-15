#include "rastreador_agregado_origen.h"

#include <unordered_set>

namespace sdi {

namespace {
constexpr double VENTANA_CORTA_SEGUNDOS = 5.0;
constexpr double VENTANA_MEDIA_SEGUNDOS = 60.0;
constexpr double VENTANA_LARGA_SEGUNDOS = 300.0;
constexpr double VENTANA_FLUJO_ACTIVO_SEGUNDOS = 60.0;
}

std::string RastreadorAgregadoOrigen::construir_clave_flujo(const EventoRed& evento) const {
    return evento.ip_origen + ":" + std::to_string(evento.puerto_origen) + "->" +
           evento.ip_destino + ":" + std::to_string(evento.puerto_destino) + "-" +
           std::to_string(evento.protocolo);
}

MetricasAgregadasOrigen RastreadorAgregadoOrigen::registrar_y_calcular(const EventoRed& evento,
                                                                        double instante_actual) {
    MetricasAgregadasOrigen metricas;

    std::lock_guard<std::mutex> bloqueo(mutex_);

    std::string clave_flujo = construir_clave_flujo(evento);
    auto it_actividad = ultima_actividad_por_flujo_.find(clave_flujo);
    bool flujo_ya_activo = it_actividad != ultima_actividad_por_flujo_.end() &&
                           (instante_actual - it_actividad->second) <= VENTANA_FLUJO_ACTIVO_SEGUNDOS;

    ultima_actividad_por_flujo_[clave_flujo] = instante_actual;

    auto& historial = historiales_[evento.ip_origen];

    if (!flujo_ya_activo) {
        historial.push_back(RegistroConexion{instante_actual, evento.ip_destino, evento.puerto_destino,
                                              evento.protocolo});
    }

    while (!historial.empty() && instante_actual - historial.front().instante > VENTANA_LARGA_SEGUNDOS) {
        historial.pop_front();
    }

    std::unordered_set<int> puertos_5s;
    std::unordered_set<std::string> ips_60s;

    for (const auto& registro : historial) {
        double antiguedad = instante_actual - registro.instante;

        if (antiguedad <= VENTANA_CORTA_SEGUNDOS) {
            metricas.conexiones_5s++;
            puertos_5s.insert(registro.puerto_destino);
        }

        if (antiguedad <= VENTANA_MEDIA_SEGUNDOS) {
            ips_60s.insert(registro.ip_destino);
        }

        if (antiguedad <= VENTANA_LARGA_SEGUNDOS &&
            registro.ip_destino == evento.ip_destino &&
            registro.puerto_destino == evento.puerto_destino &&
            registro.protocolo == evento.protocolo) {
            metricas.conexiones_mismo_destino_300s++;
        }
    }

    metricas.puertos_distintos_5s = static_cast<int>(puertos_5s.size());
    metricas.ips_distintas_60s = static_cast<int>(ips_60s.size());

    return metricas;
}

}