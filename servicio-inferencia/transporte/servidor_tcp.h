#pragma once

#include <functional>
#include <string>

#include "compatibilidad_sockets.h"

namespace sdi {

class ServidorTcp {
public:
    using ManejadorLinea = std::function<void(const std::string&)>;

    explicit ServidorTcp(int puerto);
    ~ServidorTcp();

    bool iniciar(std::string* mensaje_error);
    void ejecutar(const ManejadorLinea& manejador);
    void detener();

private:
    tipo_socket aceptar_con_tiempo_limite(int tiempo_limite_ms);

    int puerto_;
    tipo_socket descriptor_escucha_ = SOCKET_INVALIDO;
};

}
