#pragma once

#include <string>

#include "detector_aprendizaje_automatico.h"
#include "evento_red.h"
#include "interfaz_clasificador_eventos.h"

namespace sdi {

class IRegistradorEventos {
public:
    virtual ~IRegistradorEventos() = default;
    virtual void registrar(const EventoRed& evento, const VeredictoClasificacion& veredicto,
                            const std::string& clasificador, double tiempo_respuesta_ms,
                            const DiagnosticoIA& veredicto_ia_diagnostico) = 0;
};

}