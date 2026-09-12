#include "canalizador_eventos.h"

#include <chrono>

#include "reloj.h"

namespace sdi {

namespace {
constexpr int PROTOCOLO_TCP = 6;
constexpr int PROTOCOLO_UDP = 17;

bool es_inicio_conexion(const EventoRed& evento) {
    if (evento.protocolo == PROTOCOLO_TCP) {
        return evento.es_syn;
    }
    if (evento.protocolo == PROTOCOLO_UDP) {
        return true;
    }
    return false;
}
}

CanalizadorEventos::CanalizadorEventos(std::vector<IClasificadorEventos*> clasificadores,
                                        INotificadorAlertas& notificador, IRegistradorEventos& registrador,
                                        DetectorAprendizajeAutomatico* detector_diagnostico_ia)
    : clasificadores_(std::move(clasificadores)),
      notificador_(notificador),
      registrador_(registrador),
      detector_diagnostico_ia_(detector_diagnostico_ia) {}

void CanalizadorEventos::procesar(const EventoRed& evento_entrante) {
    auto inicio = std::chrono::steady_clock::now();
    total_procesado_++;

    EventoRed evento = evento_entrante;

    if (es_inicio_conexion(evento)) {
        MetricasAgregadasOrigen metricas = agregador_origen_.registrar_y_calcular(evento, tiempo::segundos_actuales());
        evento.conexiones_origen_5s = metricas.conexiones_5s;
        evento.puertos_distintos_origen_5s = metricas.puertos_distintos_5s;
        evento.ips_distintas_origen_60s = metricas.ips_distintas_60s;
        evento.conexiones_mismo_destino_300s = metricas.conexiones_mismo_destino_300s;
    }

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

    VeredictoClasificacion veredicto_ia_diagnostico;
    if (detector_diagnostico_ia_ != nullptr) {
        veredicto_ia_diagnostico = detector_diagnostico_ia_->diagnosticar(evento);
    }

    auto fin = std::chrono::steady_clock::now();
    double tiempo_respuesta_ms = std::chrono::duration<double, std::milli>(fin - inicio).count();

    registrador_.registrar(evento, veredicto_final, clasificador_nombre, tiempo_respuesta_ms, veredicto_ia_diagnostico);

    if (veredicto_final.es_amenaza) {
        total_alertas_++;
        notificador_.notificar(evento, veredicto_final, tiempo_respuesta_ms);
    }
}

size_t CanalizadorEventos::total_procesado() const { return total_procesado_.load(); }
size_t CanalizadorEventos::total_alertas() const { return total_alertas_.load(); }

}