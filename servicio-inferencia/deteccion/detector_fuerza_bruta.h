#pragma once

#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "interfaz_clasificador_eventos.h"

namespace sdi {

class DetectorFuerzaBruta : public IClasificadorEventos {
public:
    DetectorFuerzaBruta(int umbral_intentos, double ventana_segundos);

    VeredictoClasificacion clasificar(const EventoRed& evento) override;
    std::string nombre() const override;

private:
    struct ContadorIntentos {
        std::deque<double> marcas_tiempo;
        double ultima_alerta = 0.0;
    };

    bool es_ip_privada(const std::string& ip) const;
    bool es_puerto_credencial(int puerto) const;

    int umbral_intentos_;
    double ventana_segundos_;
    double cooldown_alerta_segundos_;
    std::unordered_set<int> puertos_credenciales_;

    std::mutex mutex_;
    std::unordered_map<std::string, ContadorIntentos> contadores_;
};

}