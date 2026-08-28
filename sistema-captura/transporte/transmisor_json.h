#pragma once

#include <string>
#include <vector>

#include "evento_red.h"
#include "interfaz_transmisor_eventos.h"

namespace sdi {

class TransmisorJson : public ITransmisorEventos {
public:
    TransmisorJson(std::string destino, int puerto);
    ~TransmisorJson() override;

    bool enviar(const std::vector<EventoRed>& lote) override;
    void cerrar() override;

private:
    bool asegurar_conexion();

    std::string destino_;
    int puerto_;
    int descriptor_socket_ = -1;
};

}
