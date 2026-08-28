#pragma once

#include <vector>

#include "evento_red.h"

namespace sdi {

class ITransmisorEventos {
public:
    virtual ~ITransmisorEventos() = default;
    virtual bool enviar(const std::vector<EventoRed>& lote) = 0;
    virtual void cerrar() = 0;
};

}
