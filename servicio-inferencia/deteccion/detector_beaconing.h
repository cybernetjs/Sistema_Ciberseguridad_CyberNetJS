#pragma once

#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>

#include "interfaz_clasificador_eventos.h"

namespace sdi {

class DetectorBeaconing : public IClasificadorEventos {
public:
    DetectorBeaconing(int minimo_repeticiones, double intervalo_min_segundos, double intervalo_max_segundos,
                       double variacion_maxima_permitida);

    VeredictoClasificacion clasificar(const EventoRed& evento) override;
    std::string nombre() const override;

private:
    struct Historial {
        std::deque<double> tiempos;
        long bytes_acumulados = 0;
        bool ya_alertado = false;
    };

    bool es_multicast_o_broadcast(const std::string& ip) const;

    int minimo_repeticiones_;
    double intervalo_min_segundos_;
    double intervalo_max_segundos_;
    double variacion_maxima_permitida_;

    std::mutex mutex_;
    std::unordered_map<std::string, Historial> historiales_;
};

}