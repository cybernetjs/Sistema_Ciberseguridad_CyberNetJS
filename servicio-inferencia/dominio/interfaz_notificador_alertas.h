#pragma once

#include "evento_red.h"
#include "interfaz_clasificador_eventos.h"

namespace sdi {

class INotificadorAlertas {
public:
    virtual ~INotificadorAlertas() = default;
    virtual void notificar(const EventoRed& evento, const VeredictoClasificacion& veredicto,
                            double tiempo_respuesta_ms) = 0;
};

}
