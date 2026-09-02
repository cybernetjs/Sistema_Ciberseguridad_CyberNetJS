#include "detector_fuerza_bruta.h"

#include "reloj.h"

namespace sdi {

namespace {
constexpr int PROTOCOLO_TCP = 6;
}

DetectorFuerzaBruta::DetectorFuerzaBruta(int umbral_intentos, double ventana_segundos)
    : umbral_intentos_(umbral_intentos),
      ventana_segundos_(ventana_segundos),
      puertos_credenciales_{21, 22, 23, 25, 110, 143, 445, 1433, 3306, 3389, 5900} {}

std::string DetectorFuerzaBruta::nombre() const { return "fuerza_bruta"; }

bool DetectorFuerzaBruta::es_ip_privada(const std::string& ip) const {
    if (ip.rfind("192.168.", 0) == 0) return true;
    if (ip.rfind("10.", 0) == 0) return true;
    if (ip.rfind("172.16.", 0) == 0) return true;
    if (ip.rfind("172.17.", 0) == 0) return true;
    if (ip.rfind("172.18.", 0) == 0) return true;
    if (ip.rfind("172.19.", 0) == 0) return true;
    if (ip.rfind("172.2", 0) == 0) return true;
    if (ip.rfind("172.30.", 0) == 0) return true;
    if (ip.rfind("172.31.", 0) == 0) return true;
    return false;
}

bool DetectorFuerzaBruta::es_puerto_credencial(int puerto) const {
    return puertos_credenciales_.count(puerto) > 0;
}

VeredictoClasificacion DetectorFuerzaBruta::clasificar(const EventoRed& evento) {
    if (evento.protocolo != PROTOCOLO_TCP || !evento.es_syn) {
        return VeredictoClasificacion{};
    }

    if (!es_ip_privada(evento.ip_destino)) {
        return VeredictoClasificacion{};
    }

    if (!es_puerto_credencial(evento.puerto_destino)) {
        return VeredictoClasificacion{};
    }

    std::string clave = evento.ip_origen + "->" + evento.ip_destino + ":" + std::to_string(evento.puerto_destino);

    double instante_actual = tiempo::segundos_actuales();
    std::lock_guard<std::mutex> bloqueo(mutex_);

    auto& contador = contadores_[clave];
    if (instante_actual - contador.inicio_ventana > ventana_segundos_) {
        contador.inicio_ventana = instante_actual;
        contador.cantidad = 0;
        contador.ya_alertado = false;
    }

    contador.cantidad += 1;

    if (contador.cantidad > umbral_intentos_ && !contador.ya_alertado) {
        contador.ya_alertado = true;
        return VeredictoClasificacion{true,
                                       "Posible fuerza bruta de credenciales desde " + evento.ip_origen +
                                           " hacia " + evento.ip_destino + ":" + std::to_string(evento.puerto_destino),
                                       0.85};
    }

    return VeredictoClasificacion{};
}

}