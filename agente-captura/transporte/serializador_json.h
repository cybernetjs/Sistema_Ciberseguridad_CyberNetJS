#pragma once

#include <string>
#include <vector>

#include "evento_red.h"

namespace sdi {

std::string evento_a_json(const EventoRed& evento);
std::string lote_a_json(const std::vector<EventoRed>& lote);

}
