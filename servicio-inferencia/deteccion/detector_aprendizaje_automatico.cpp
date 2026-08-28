#include "detector_aprendizaje_automatico.h"

namespace sdi {

bool DetectorAprendizajeAutomatico::cargar_modelo(const std::string& ruta_modelo) {
    ruta_modelo_ = ruta_modelo;
    modelo_cargado_ = false;
    return false;
}

VeredictoClasificacion DetectorAprendizajeAutomatico::clasificar(const EventoRed&) {
    return VeredictoClasificacion{};
}

std::string DetectorAprendizajeAutomatico::nombre() const { return "aprendizaje_automatico"; }

}
