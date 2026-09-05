#include "detector_fuerza_bruta.h"

#include "reloj.h"

namespace sdi {

namespace {
constexpr int PROTOCOLO_TCP = 6;
}

DetectorFuerzaBruta::DetectorFuerzaBruta(int umbral_intentos, double ventana_segundos)
    : umbral_intentos_(umbral_intentos),
      ventana_segundos_(ventana_segundos),
      cooldown_alerta_segundos_(60.0),
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
    contador.marcas_tiempo.push_back(instante_actual);

    while (!contador.marcas_tiempo.empty() &&
           instante_actual - contador.marcas_tiempo.front() > ventana_segundos_) {
        contador.marcas_tiempo.pop_front();
    }

    bool supera_umbral = static_cast<int>(contador.marcas_tiempo.size()) > umbral_intentos_;
    bool paso_cooldown = instante_actual - contador.ultima_alerta > cooldown_alerta_segundos_;

    if (supera_umbral && paso_cooldown) {
        contador.ultima_alerta = instante_actual;
        return VeredictoClasificacion{true,
                                       "Posible fuerza bruta de credenciales desde " + evento.ip_origen +
                                           " hacia " + evento.ip_destino + ":" + std::to_string(evento.puerto_destino),
                                       0.85};
    }

    return VeredictoClasificacion{};
}

}