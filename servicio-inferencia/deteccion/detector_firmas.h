#pragma once

#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include "interfaz_clasificador_eventos.h"

namespace sdi {

class DetectorFirmas : public IClasificadorEventos {
public:
    DetectorFirmas(int umbral_paquetes_por_segundo, double ventana_segundos);

    VeredictoClasificacion clasificar(const EventoRed& evento) override;
    std::string nombre() const override;

private:
    struct ContadorInundacion {
        int cantidad = 0;
        double inicio_ventana = 0.0;
        bool ya_alertado = false;
    };

    int umbral_paquetes_por_segundo_;
    double ventana_segundos_;
    std::unordered_set<int> puertos_ataque_conocidos_;

    std::mutex mutex_;
    std::unordered_map<std::string, ContadorInundacion> contadores_inundacion_;
};

}
