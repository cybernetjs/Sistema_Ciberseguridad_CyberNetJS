#pragma once

#include <atomic>
#include <vector>

#include "detector_aprendizaje_automatico.h"
#include "evento_red.h"
#include "interfaz_clasificador_eventos.h"
#include "interfaz_notificador_alertas.h"
#include "interfaz_registrador_eventos.h"
#include "rastreador_agregado_origen.h"

namespace sdi {

class CanalizadorEventos {
public:
    CanalizadorEventos(std::vector<IClasificadorEventos*> clasificadores, INotificadorAlertas& notificador,
                        IRegistradorEventos& registrador, DetectorAprendizajeAutomatico* detector_diagnostico_ia);

    void procesar(const EventoRed& evento);

    size_t total_procesado() const;
    size_t total_alertas() const;

private:
    std::vector<IClasificadorEventos*> clasificadores_;
    INotificadorAlertas& notificador_;
    IRegistradorEventos& registrador_;
    DetectorAprendizajeAutomatico* detector_diagnostico_ia_;
    RastreadorAgregadoOrigen agregador_origen_;

    std::atomic<size_t> total_procesado_{0};
    std::atomic<size_t> total_alertas_{0};
};

}