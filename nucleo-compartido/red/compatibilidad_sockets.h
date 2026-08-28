#pragma once

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

namespace sdi {

using tipo_socket = SOCKET;
using descriptor_sondeo = WSAPOLLFD;

constexpr tipo_socket SOCKET_INVALIDO = INVALID_SOCKET;

inline bool inicializar_sockets() {
    WSADATA datos_wsa;
    return WSAStartup(MAKEWORD(2, 2), &datos_wsa) == 0;
}

inline void finalizar_sockets() { WSACleanup(); }

inline int cerrar_socket(tipo_socket s) { return closesocket(s); }

inline int esperar_socket(descriptor_sondeo* descriptores, unsigned long cantidad, int tiempo_limite_ms) {
    return WSAPoll(descriptores, cantidad, tiempo_limite_ms);
}

}

#else

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace sdi {

using tipo_socket = int;
using descriptor_sondeo = struct pollfd;

constexpr tipo_socket SOCKET_INVALIDO = -1;

inline bool inicializar_sockets() { return true; }

inline void finalizar_sockets() {}

inline int cerrar_socket(tipo_socket s) { return close(s); }

inline int esperar_socket(descriptor_sondeo* descriptores, nfds_t cantidad, int tiempo_limite_ms) {
    return poll(descriptores, cantidad, tiempo_limite_ms);
}

}

#endif
