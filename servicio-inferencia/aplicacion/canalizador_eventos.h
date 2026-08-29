#pragma once

#include <atomic>
#include <vector>

#include "evento_red.h"
#include "interfaz_clasificador_eventos.h"
#include "interfaz_notificador_alertas.h"
#include "interfaz_registrador_eventos.h"

namespace sdi {

class CanalizadorEventos {
public:
    CanalizadorEventos(std::vector<IClasificadorEventos*> clasificadores, INotificadorAlertas& notificador,
                        IRegistradorEventos& registrador);

    void procesar(const EventoRed& evento);

    size_t total_procesado() const;
    size_t total_alertas() const;

private:
    std::vector<IClasificadorEventos*> clasificadores_;
    INotificadorAlertas& notificador_;
    IRegistradorEventos& registrador_;

    std::atomic<size_t> total_procesado_{0};
    std::atomic<size_t> total_alertas_{0};
};

}
