#include "detector_reconocimiento.h"

#include "reloj.h"

namespace sdi {

namespace {
constexpr int PROTOCOLO_TCP = 6;
constexpr int PROTOCOLO_UDP = 17;
}

DetectorReconocimiento::DetectorReconocimiento(int umbral_puertos_distintos, double ventana_segundos)
    : umbral_puertos_distintos_(umbral_puertos_distintos), ventana_segundos_(ventana_segundos) {}

std::string DetectorReconocimiento::nombre() const { return "reconocimiento"; }

VeredictoClasificacion DetectorReconocimiento::clasificar(const EventoRed& evento) {
    if (evento.protocolo != PROTOCOLO_TCP && evento.protocolo != PROTOCOLO_UDP) {
        return VeredictoClasificacion{};
    }

    double instante_actual = tiempo::segundos_actuales();
    std::lock_guard<std::mutex> bloqueo(mutex_);

    auto& contador = contadores_[evento.ip_origen];
    if (instante_actual - contador.inicio_ventana > ventana_segundos_) {
        contador.inicio_ventana = instante_actual;
        contador.puertos_vistos.clear();
        contador.ya_alertado = false;
    }

    contador.puertos_vistos.insert(evento.puerto_destino);

    if (static_cast<int>(contador.puertos_vistos.size()) > umbral_puertos_distintos_ && !contador.ya_alertado) {
        contador.ya_alertado = true;
        return VeredictoClasificacion{true, "Posible escaneo de puertos desde " + evento.ip_origen, 0.85};
    }

    return VeredictoClasificacion{};
}

}