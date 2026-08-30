#include "detector_dga.h"

#include <algorithm>
#include <cctype>

#include "reloj.h"

namespace sdi {

DetectorDga::DetectorDga(int umbral_dominios_sospechosos, double ventana_segundos)
    : umbral_dominios_sospechosos_(umbral_dominios_sospechosos), ventana_segundos_(ventana_segundos) {}

std::string DetectorDga::nombre() const { return "dga"; }

bool DetectorDga::es_dominio_sospechoso(const std::string& dominio) const {
    std::string etiqueta = dominio;
    auto ultimo_punto = etiqueta.rfind('.');
    if (ultimo_punto != std::string::npos) {
        etiqueta = etiqueta.substr(0, ultimo_punto);
    }
    auto penultimo_punto = etiqueta.rfind('.');
    if (penultimo_punto != std::string::npos) {
        etiqueta = etiqueta.substr(penultimo_punto + 1);
    }

    if (etiqueta.size() < 10) {
        return false;
    }

    int vocales = 0;
    int digitos = 0;
    int consonantes_seguidas = 0;
    int max_consonantes_seguidas = 0;

    for (char c : etiqueta) {
        char l = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (l == 'a' || l == 'e' || l == 'i' || l == 'o' || l == 'u') {
            vocales++;
            consonantes_seguidas = 0;
        } else if (std::isdigit(static_cast<unsigned char>(l))) {
            digitos++;
            consonantes_seguidas = 0;
        } else if (std::isalpha(static_cast<unsigned char>(l))) {
            consonantes_seguidas++;
            max_consonantes_seguidas = std::max(max_consonantes_seguidas, consonantes_seguidas);
        }
    }

    double fraccion_vocales = static_cast<double>(vocales) / static_cast<double>(etiqueta.size());
    double fraccion_digitos = static_cast<double>(digitos) / static_cast<double>(etiqueta.size());

    return (fraccion_vocales < 0.25) || (fraccion_digitos > 0.3) || (max_consonantes_seguidas >= 5);
}

VeredictoClasificacion DetectorDga::clasificar(const EventoRed& evento) {
    if (evento.consulta_dns.empty()) {
        return VeredictoClasificacion{};
    }

    if (!es_dominio_sospechoso(evento.consulta_dns)) {
        return VeredictoClasificacion{};
    }

    double instante_actual = tiempo::segundos_actuales();
    std::lock_guard<std::mutex> bloqueo(mutex_);

    auto& contador = contadores_[evento.ip_origen];
    if (instante_actual - contador.inicio_ventana > ventana_segundos_) {
        contador.inicio_ventana = instante_actual;
        contador.dominios_sospechosos.clear();
        contador.ya_alertado = false;
    }

    contador.dominios_sospechosos.insert(evento.consulta_dns);

    if (static_cast<int>(contador.dominios_sospechosos.size()) > umbral_dominios_sospechosos_ && !contador.ya_alertado) {
        contador.ya_alertado = true;
        return VeredictoClasificacion{true, "Posible generacion algoritmica de dominios (DGA) desde " + evento.ip_origen, 0.75};
    }

    return VeredictoClasificacion{};
}

}