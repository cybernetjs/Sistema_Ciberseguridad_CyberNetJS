#pragma once

#include <atomic>
#include <string>

#include "interfaz_clasificador_eventos.h"

namespace sdi {

class DetectorAprendizajeAutomatico : public IClasificadorEventos {
public:
    bool cargar_modelo(const std::string& ruta_modelo);

    VeredictoClasificacion clasificar(const EventoRed& evento) override;
    std::string nombre() const override;

private:
    std::atomic<bool> modelo_cargado_{false};
    std::string ruta_modelo_;
};

}
