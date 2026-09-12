#pragma once

#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>

#include "evento_red.h"

namespace sdi {

struct MetricasAgregadasOrigen {
    int conexiones_5s = 0;
    int puertos_distintos_5s = 0;
    int ips_distintas_60s = 0;
    int conexiones_mismo_destino_300s = 0;
};

class RastreadorAgregadoOrigen {
public:
    MetricasAgregadasOrigen registrar_y_calcular(const EventoRed& evento, double instante_actual);

private:
    struct RegistroConexion {
        double instante = 0.0;
        std::string ip_destino;
        int puerto_destino = 0;
        int protocolo = 0;
    };

    std::mutex mutex_;
    std::unordered_map<std::string, std::deque<RegistroConexion>> historiales_;
};

}