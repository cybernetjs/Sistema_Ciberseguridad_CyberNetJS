#include "canalizador_eventos.h"

#include <chrono>

namespace sdi {

CanalizadorEventos::CanalizadorEventos(std::vector<IClasificadorEventos*> clasificadores,
                                        INotificadorAlertas& notificador, IRegistradorEventos& registrador)
    : clasificadores_(std::move(clasificadores)), notificador_(notificador), registrador_(registrador) {}

void CanalizadorEventos::procesar(const EventoRed& evento) {
    auto inicio = std::chrono::steady_clock::now();
    total_procesado_++;

    VeredictoClasificacion veredicto_final;
    std::string clasificador_nombre = "ninguno";

    for (auto* clasificador : clasificadores_) {
        VeredictoClasificacion veredicto = clasificador->clasificar(evento);
        if (veredicto.es_amenaza) {
            veredicto_final = veredicto;
            clasificador_nombre = clasificador->nombre();
            break;
        }
    }

    auto fin = std::chrono::steady_clock::now();
    double tiempo_respuesta_ms = std::chrono::duration<double, std::milli>(fin - inicio).count();

    registrador_.registrar(evento, veredicto_final, clasificador_nombre, tiempo_respuesta_ms);

    if (veredicto_final.es_amenaza) {
        total_alertas_++;
        notificador_.notificar(evento, veredicto_final, tiempo_respuesta_ms);
    }
}

size_t CanalizadorEventos::total_procesado() const { return total_procesado_.load(); }
size_t CanalizadorEventos::total_alertas() const { return total_alertas_.load(); }

}
