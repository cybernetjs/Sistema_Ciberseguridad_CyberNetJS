#pragma once

#include <string>
#include <vector>

#include "evento_red.h"

namespace sdi {

std::vector<EventoRed> decodificar_eventos(const std::string& linea);

}
