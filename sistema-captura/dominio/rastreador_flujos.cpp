#include "rastreador_flujos.h"

#include <algorithm>

#include "reloj.h"

namespace sdi {

RastreadorFlujos::RastreadorFlujos(double edad_maxima_segundos, std::chrono::seconds intervalo_limpieza)
    : inicio_(tiempo::segundos_actuales()),
      edad_maxima_segundos_(edad_maxima_segundos),
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

double RastreadorFlujos::actualizar_y_obtener_duracion(const ClaveFlujo& clave, double instante_actual) {
    std::lock_guard<std::mutex> bloqueo(mutex_mapa_);
    auto encontrado = ultima_actividad_.find(clave);
    double anterior = (encontrado != ultima_actividad_.end()) ? encontrado->second : inicio_;
    double duracion = std::max(0.001, instante_actual - anterior);
    ultima_actividad_[clave] = instante_actual;
    return duracion;
}

size_t RastreadorFlujos::flujos_rastreados() const {
    std::lock_guard<std::mutex> bloqueo(mutex_mapa_);
    return ultima_actividad_.size();
}

void RastreadorFlujos::limpiar_una_vez(double instante_actual) {
    std::lock_guard<std::mutex> bloqueo(mutex_mapa_);
    for (auto it = ultima_actividad_.begin(); it != ultima_actividad_.end();) {
        if (instante_actual - it->second > edad_maxima_segundos_) {
            it = ultima_actividad_.erase(it);
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
