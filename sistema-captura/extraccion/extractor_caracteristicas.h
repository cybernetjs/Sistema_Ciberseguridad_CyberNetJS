#pragma once

#include <cstdint>
#include <optional>

#include "evento_red.h"
#include "rastreador_flujos.h"

namespace sdi {

std::optional<EventoRed> extraer_caracteristicas(const uint8_t* paquete,
                                                   uint32_t longitud_capturada,
                                                   RastreadorFlujos& rastreador,
                                                   int tipo_enlace);

}
