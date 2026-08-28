#pragma once

#include <string>

#include "evento_red.h"

namespace sdi {

struct VeredictoClasificacion {
    bool es_amenaza = false;
    std::string etiqueta;
    double confianza = 0.0;
};

class IClasificadorEventos {
public:
    virtual ~IClasificadorEventos() = default;
    virtual VeredictoClasificacion clasificar(const EventoRed& evento) = 0;
    virtual std::string nombre() const = 0;
};

}
