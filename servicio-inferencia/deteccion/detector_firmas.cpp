#include "detector_firmas.h"

#include "reloj.h"

namespace sdi {

namespace {
constexpr int PROTOCOLO_TCP = 6;
}

DetectorFirmas::DetectorFirmas(int umbral_paquetes_por_segundo, double ventana_segundos)
    : umbral_paquetes_por_segundo_(umbral_paquetes_por_segundo),
      ventana_segundos_(ventana_segundos),
      puertos_ataque_conocidos_{23, 2323, 7547, 5555, 37215, 8291, 9527} {}

std::string DetectorFirmas::nombre() const { return "firmas"; }

VeredictoClasificacion DetectorFirmas::clasificar(const EventoRed& evento) {
    if (puertos_ataque_conocidos_.count(evento.puerto_destino) > 0) {
        return VeredictoClasificacion{true,
                                       "Acceso a puerto conocido de explotacion IoT (" +
                                           std::to_string(evento.puerto_destino) + ")",
                                       1.0};
    }

    if (evento.protocolo == PROTOCOLO_TCP && !evento.es_syn) {
        return VeredictoClasificacion{};
    }

    double instante_actual = tiempo::segundos_actuales();
    std::lock_guard<std::mutex> bloqueo(mutex_);

    auto& contador = contadores_inundacion_[evento.ip_origen];
    if (instante_actual - contador.inicio_ventana > ventana_segundos_) {
        contador.inicio_ventana = instante_actual;
        contador.cantidad = 0;
        contador.ya_alertado = false;
    }
    contador.cantidad += 1;

    if (contador.cantidad > umbral_paquetes_por_segundo_ && !contador.ya_alertado) {
        contador.ya_alertado = true;
        return VeredictoClasificacion{true, "Posible flooding DoS/DDoS desde " + evento.ip_origen, 0.9};
    }

    return VeredictoClasificacion{};
}

}