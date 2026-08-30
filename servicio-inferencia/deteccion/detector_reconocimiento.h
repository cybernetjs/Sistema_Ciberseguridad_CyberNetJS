#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "interfaz_clasificador_eventos.h"

namespace sdi {

class DetectorReconocimiento : public IClasificadorEventos {
public:
    DetectorReconocimiento(int umbral_puertos_distintos, double ventana_segundos);

    VeredictoClasificacion clasificar(const EventoRed& evento) override;
    std::string nombre() const override;

private:
    struct ContadorEscaneo {
        std::unordered_set<int> puertos_vistos;
        double inicio_ventana = 0.0;
        bool ya_alertado = false;
    };

    int umbral_puertos_distintos_;
    double ventana_segundos_;

    std::mutex mutex_;
    std::unordered_map<std::string, ContadorEscaneo> contadores_;
};

}