#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "interfaz_clasificador_eventos.h"

namespace sdi {

class DetectorDga : public IClasificadorEventos {
public:
    DetectorDga(int umbral_dominios_sospechosos, double ventana_segundos);

    VeredictoClasificacion clasificar(const EventoRed& evento) override;
    std::string nombre() const override;

private:
    struct ContadorDga {
        std::unordered_set<std::string> dominios_sospechosos;
        double inicio_ventana = 0.0;
        bool ya_alertado = false;
    };

    bool es_dominio_sospechoso(const std::string& dominio) const;

    int umbral_dominios_sospechosos_;
    double ventana_segundos_;

    std::mutex mutex_;
    std::unordered_map<std::string, ContadorDga> contadores_;
};

}