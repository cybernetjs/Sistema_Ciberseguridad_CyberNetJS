#pragma once

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
        int cantidad = 0;
        double inicio_ventana = 0.0;
        bool ya_alertado = false;
    };

    bool es_ip_privada(const std::string& ip) const;
    bool es_puerto_credencial(int puerto) const;

    int umbral_intentos_;
    double ventana_segundos_;
    std::unordered_set<int> puertos_credenciales_;

    std::mutex mutex_;
    std::unordered_map<std::string, ContadorIntentos> contadores_;
};

}