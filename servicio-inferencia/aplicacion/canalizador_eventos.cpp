#include "canalizador_eventos.h"

#include <chrono>

namespace sdi {

CanalizadorEventos::CanalizadorEventos(std::vector<IClasificadorEventos*> clasificadores,
                                        INotificadorAlertas& notificador)
    : clasificadores_(std::move(clasificadores)), notificador_(notificador) {}

void CanalizadorEventos::procesar(const EventoRed& evento) {
    auto inicio = std::chrono::steady_clock::now();
    total_procesado_++;

    for (auto* clasificador : clasificadores_) {
        VeredictoClasificacion veredicto = clasificador->clasificar(evento);
        if (veredicto.es_amenaza) {
            auto fin = std::chrono::steady_clock::now();
            double tiempo_respuesta_ms = std::chrono::duration<double, std::milli>(fin - inicio).count();
            total_alertas_++;
            notificador_.notificar(evento, veredicto, tiempo_respuesta_ms);
            return;
        }
    }
}

size_t CanalizadorEventos::total_procesado() const { return total_procesado_.load(); }
size_t CanalizadorEventos::total_alertas() const { return total_alertas_.load(); }

}
