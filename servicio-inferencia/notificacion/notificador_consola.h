#pragma once

#include "interfaz_notificador_alertas.h"

namespace sdi {

class NotificadorConsola : public INotificadorAlertas {
public:
    void notificar(const EventoRed& evento, const VeredictoClasificacion& veredicto,
                    double tiempo_respuesta_ms) override;
};

}
