#include "transmisor_json.h"

#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include "controlador_apagado.h"
#include "serializador_json.h"

namespace sdi {

namespace {

constexpr int TIEMPO_LIMITE_CONEXION_MS = 2000;
constexpr int PASO_SONDEO_CONEXION_MS = 200;
constexpr int TIEMPO_LIMITE_ES_MS = 10;
constexpr int INTENTOS_MAXIMOS_ENVIO = 3;
constexpr int RETARDO_REINTENTO_MS = 250;

}

TransmisorJson::TransmisorJson(std::string destino, int puerto)
    : destino_(std::move(destino)), puerto_(puerto) {}

TransmisorJson::~TransmisorJson() { cerrar(); }

bool TransmisorJson::asegurar_conexion() {
    if (descriptor_socket_ != -1) {
        return true;
    }

    struct addrinfo pistas{};
    pistas.ai_family = AF_INET;
    pistas.ai_socktype = SOCK_STREAM;

    struct addrinfo* resultado = nullptr;
    std::string puerto_texto = std::to_string(puerto_);
    if (getaddrinfo(destino_.c_str(), puerto_texto.c_str(), &pistas, &resultado) != 0) {
        return false;
    }

    int descriptor = socket(resultado->ai_family, resultado->ai_socktype, resultado->ai_protocol);
    if (descriptor < 0) {
        freeaddrinfo(resultado);
        return false;
    }

    int banderas = fcntl(descriptor, F_GETFL, 0);
    fcntl(descriptor, F_SETFL, banderas | O_NONBLOCK);

    int codigo_conexion = connect(descriptor, resultado->ai_addr, resultado->ai_addrlen);
    freeaddrinfo(resultado);

    bool conectado = false;

    if (codigo_conexion == 0) {
        conectado = true;
    } else if (errno == EINPROGRESS) {
        int transcurrido_ms = 0;
        while (transcurrido_ms < TIEMPO_LIMITE_CONEXION_MS) {
            if (!ControladorApagado::instancia().debe_continuar() && transcurrido_ms > 0) {
                break;
            }
            struct pollfd descriptor_sondeo{};
            descriptor_sondeo.fd = descriptor;
            descriptor_sondeo.events = POLLOUT;
            int resultado_sondeo = poll(&descriptor_sondeo, 1, PASO_SONDEO_CONEXION_MS);
            if (resultado_sondeo > 0 && (descriptor_sondeo.revents & (POLLOUT | POLLERR | POLLHUP))) {
                int codigo_error = 0;
                socklen_t longitud = sizeof(codigo_error);
                getsockopt(descriptor, SOL_SOCKET, SO_ERROR, &codigo_error, &longitud);
                conectado = (codigo_error == 0);
                break;
            }
            transcurrido_ms += PASO_SONDEO_CONEXION_MS;
        }
    }

    if (!conectado) {
        ::close(descriptor);
        return false;
    }

    fcntl(descriptor, F_SETFL, banderas);

    struct timeval tiempo_limite{};
    tiempo_limite.tv_sec = TIEMPO_LIMITE_ES_MS;
    setsockopt(descriptor, SOL_SOCKET, SO_SNDTIMEO, &tiempo_limite, sizeof(tiempo_limite));
    setsockopt(descriptor, SOL_SOCKET, SO_RCVTIMEO, &tiempo_limite, sizeof(tiempo_limite));

    descriptor_socket_ = descriptor;
    return true;
}

void TransmisorJson::cerrar() {
    if (descriptor_socket_ != -1) {
        ::close(descriptor_socket_);
        descriptor_socket_ = -1;
    }
}

bool TransmisorJson::enviar(const std::vector<EventoRed>& lote) {
    if (lote.empty()) {
        return true;
    }

    std::string linea = lote_a_json(lote) + "\n";

    for (int intento = 0; intento < INTENTOS_MAXIMOS_ENVIO; ++intento) {
        if (!asegurar_conexion()) {
            cerrar();
            std::this_thread::sleep_for(std::chrono::milliseconds(RETARDO_REINTENTO_MS));
            continue;
        }

        size_t total_enviado = 0;
        const char* datos = linea.data();
        size_t longitud = linea.size();
        bool exito = true;

        while (total_enviado < longitud) {
            ssize_t enviados = ::send(descriptor_socket_, datos + total_enviado, longitud - total_enviado, 0);
            if (enviados <= 0) {
                exito = false;
                break;
            }
            total_enviado += static_cast<size_t>(enviados);
        }

        if (exito) {
            return true;
        }

        cerrar();
        std::this_thread::sleep_for(std::chrono::milliseconds(RETARDO_REINTENTO_MS));
    }

    return false;
}

}
