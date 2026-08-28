#include "controlador_apagado.h"

#include <csignal>

namespace sdi {

ControladorApagado& ControladorApagado::instancia() {
    static ControladorApagado unica;
    return unica;
}

void ControladorApagado::activar() {
    std::signal(SIGINT, &ControladorApagado::atender_senal);
    std::signal(SIGTERM, &ControladorApagado::atender_senal);
}

bool ControladorApagado::debe_continuar() const { return en_ejecucion_.load(); }

void ControladorApagado::atender_senal(int) { instancia().en_ejecucion_.store(false); }

}
