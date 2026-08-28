#include "servidor_tcp.h"

#include <cstring>

#include "controlador_apagado.h"

namespace sdi {

namespace {

constexpr int ESPERA_ACEPTAR_MS = 500;
constexpr int ESPERA_RECEPCION_MS = 500;
constexpr size_t LIMITE_BYTES_LINEA = 8 * 1024 * 1024;

}

ServidorTcp::ServidorTcp(int puerto) : puerto_(puerto) {}

ServidorTcp::~ServidorTcp() { detener(); }

bool ServidorTcp::iniciar(std::string* mensaje_error) {
    descriptor_escucha_ = socket(AF_INET, SOCK_STREAM, 0);
    if (descriptor_escucha_ == SOCKET_INVALIDO) {
        if (mensaje_error) *mensaje_error = "no se pudo crear el socket";
        return false;
    }

    int opcion = 1;
    setsockopt(descriptor_escucha_, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&opcion), sizeof(opcion));

    struct sockaddr_in direccion{};
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons(static_cast<uint16_t>(puerto_));

    if (bind(descriptor_escucha_, reinterpret_cast<struct sockaddr*>(&direccion), sizeof(direccion)) != 0) {
        if (mensaje_error) *mensaje_error = "no se pudo hacer bind en el puerto " + std::to_string(puerto_);
        cerrar_socket(descriptor_escucha_);
        descriptor_escucha_ = SOCKET_INVALIDO;
        return false;
    }

    if (listen(descriptor_escucha_, 1) != 0) {
        if (mensaje_error) *mensaje_error = "no se pudo poner el socket en modo escucha";
        cerrar_socket(descriptor_escucha_);
        descriptor_escucha_ = SOCKET_INVALIDO;
        return false;
    }

    return true;
}

tipo_socket ServidorTcp::aceptar_con_tiempo_limite(int tiempo_limite_ms) {
    descriptor_sondeo descriptor{};
    descriptor.fd = descriptor_escucha_;
    descriptor.events = POLLIN;

    int resultado_sondeo = esperar_socket(&descriptor, 1, tiempo_limite_ms);
    if (resultado_sondeo <= 0) {
        return SOCKET_INVALIDO;
    }

    return accept(descriptor_escucha_, nullptr, nullptr);
}

void ServidorTcp::ejecutar(const ManejadorLinea& manejador) {
    if (descriptor_escucha_ == SOCKET_INVALIDO) {
        return;
    }

    while (ControladorApagado::instancia().debe_continuar()) {
        tipo_socket conexion = aceptar_con_tiempo_limite(ESPERA_ACEPTAR_MS);
        if (conexion == SOCKET_INVALIDO) {
            continue;
        }

        std::string acumulado;
        char fragmento[65536];

        while (ControladorApagado::instancia().debe_continuar()) {
            descriptor_sondeo descriptor{};
            descriptor.fd = conexion;
            descriptor.events = POLLIN;

            int resultado_sondeo = esperar_socket(&descriptor, 1, ESPERA_RECEPCION_MS);
            if (resultado_sondeo <= 0) {
                continue;
            }

            int recibidos = recv(conexion, fragmento, static_cast<int>(sizeof(fragmento)), 0);
            if (recibidos <= 0) {
                break;
            }

            acumulado.append(fragmento, static_cast<size_t>(recibidos));

            size_t posicion;
            while ((posicion = acumulado.find('\n')) != std::string::npos) {
                std::string linea = acumulado.substr(0, posicion);
                acumulado.erase(0, posicion + 1);
                if (!linea.empty()) {
                    manejador(linea);
                }
            }

            if (acumulado.size() > LIMITE_BYTES_LINEA) {
                acumulado.clear();
            }
        }

        cerrar_socket(conexion);
    }
}

void ServidorTcp::detener() {
    if (descriptor_escucha_ != SOCKET_INVALIDO) {
        cerrar_socket(descriptor_escucha_);
        descriptor_escucha_ = SOCKET_INVALIDO;
    }
}

}
