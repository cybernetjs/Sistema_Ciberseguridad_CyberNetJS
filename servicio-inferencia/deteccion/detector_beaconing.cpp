#include "detector_beaconing.h"

#include <cmath>
#include <numeric>

#include "reloj.h"

namespace sdi {

namespace {
constexpr int PROTOCOLO_TCP = 6;
constexpr int PROTOCOLO_UDP = 17;
constexpr size_t MAXIMO_HISTORIAL = 10;
}

DetectorBeaconing::DetectorBeaconing(int minimo_repeticiones, double intervalo_min_segundos,
                                      double intervalo_max_segundos, double variacion_maxima_permitida)
    : minimo_repeticiones_(minimo_repeticiones),
      intervalo_min_segundos_(intervalo_min_segundos),
      intervalo_max_segundos_(intervalo_max_segundos),
      variacion_maxima_permitida_(variacion_maxima_permitida) {}

std::string DetectorBeaconing::nombre() const { return "beaconing"; }

bool DetectorBeaconing::es_multicast_o_broadcast(const std::string& ip) const {
    if (ip == "255.255.255.255") {
        return true;
    }
    size_t fin_primer_octeto = ip.find('.');
    if (fin_primer_octeto == std::string::npos) {
        return false;
    }
    try {
        int primer_octeto = std::stoi(ip.substr(0, fin_primer_octeto));
        if (primer_octeto >= 224 && primer_octeto <= 239) {
            return true;
        }
    } catch (const std::exception&) {
        return false;
    }
    return false;
}

VeredictoClasificacion DetectorBeaconing::clasificar(const EventoRed& evento) {
    if (evento.protocolo != PROTOCOLO_TCP && evento.protocolo != PROTOCOLO_UDP) {
        return VeredictoClasificacion{};
    }

    if (es_multicast_o_broadcast(evento.ip_destino)) {
        return VeredictoClasificacion{};
    }

    std::string clave = evento.ip_origen + "->" + evento.ip_destino + ":" + std::to_string(evento.puerto_destino);
    double instante_actual = tiempo::segundos_actuales();

    std::lock_guard<std::mutex> bloqueo(mutex_);
    auto& historial = historiales_[clave];

    historial.tiempos.push_back(instante_actual);
    historial.bytes_acumulados += evento.bytes_origen;
    if (historial.tiempos.size() > MAXIMO_HISTORIAL) {
        historial.tiempos.pop_front();
    }

    if (historial.ya_alertado || static_cast<int>(historial.tiempos.size()) < minimo_repeticiones_) {
        return VeredictoClasificacion{};
    }

    std::vector<double> intervalos;
    for (size_t i = 1; i < historial.tiempos.size(); i++) {
        intervalos.push_back(historial.tiempos[i] - historial.tiempos[i - 1]);
    }

    double suma = std::accumulate(intervalos.begin(), intervalos.end(), 0.0);
    double media = suma / intervalos.size();

    if (media < intervalo_min_segundos_ || media > intervalo_max_segundos_) {
        return VeredictoClasificacion{};
    }

    double suma_cuadrados = 0.0;
    for (double v : intervalos) {
        suma_cuadrados += (v - media) * (v - media);
    }
    double desviacion = std::sqrt(suma_cuadrados / intervalos.size());
    double coeficiente_variacion = media > 0.0 ? desviacion / media : 1.0;

    double bytes_promedio = static_cast<double>(historial.bytes_acumulados) / static_cast<double>(historial.tiempos.size());

    if (coeficiente_variacion <= variacion_maxima_permitida_ && bytes_promedio < 5000.0) {
        historial.ya_alertado = true;
        return VeredictoClasificacion{true,
                                       "Posible beaconing C2 desde " + evento.ip_origen + " hacia " +
                                           evento.ip_destino + ":" + std::to_string(evento.puerto_destino),
                                       0.8};
    }

    return VeredictoClasificacion{};
}

}