#pragma once

#include <fstream>
#include <mutex>
#include <string>

#include "interfaz_registrador_eventos.h"

namespace sdi {

class RegistradorCsv : public IRegistradorEventos {
public:
    explicit RegistradorCsv(const std::string& ruta_archivo);

    void registrar(const EventoRed& evento, const VeredictoClasificacion& veredicto,
                    const std::string& clasificador, double tiempo_respuesta_ms) override;

    bool listo() const;

private:
    void escribir_encabezado();

    std::string ruta_archivo_;
    std::ofstream archivo_;
    std::mutex mutex_;
};

}