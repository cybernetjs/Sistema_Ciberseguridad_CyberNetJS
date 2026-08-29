#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

namespace sdi {

struct ClaveFlujo {
    std::string ip_a;
    std::string ip_b;
    int puerto_a = 0;
    int puerto_b = 0;
    int protocolo = 0;

    bool operator==(const ClaveFlujo& otra) const {
        return protocolo == otra.protocolo &&
               ip_a == otra.ip_a &&
               ip_b == otra.ip_b &&
               puerto_a == otra.puerto_a &&
               puerto_b == otra.puerto_b;
    }
};

struct HashClaveFlujo {
    size_t operator()(const ClaveFlujo& clave) const {
        size_t h1 = std::hash<std::string>{}(clave.ip_a);
        size_t h2 = std::hash<std::string>{}(clave.ip_b);
        size_t h3 = std::hash<int>{}(clave.puerto_a);
        size_t h4 = std::hash<int>{}(clave.puerto_b);
        size_t h5 = std::hash<int>{}(clave.protocolo);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4);
    }
};

struct EstadoFlujo {
    double primera_actividad = 0.0;
    double ultima_actividad = 0.0;
    std::string ip_iniciador;
    int puerto_iniciador = 0;
    long orig_pkts = 0;
    long orig_ip_bytes = 0;
    long resp_pkts = 0;
    long resp_ip_bytes = 0;
};

struct ResultadoFlujo {
    double duracion = 0.001;
    long orig_pkts = 0;
    long orig_ip_bytes = 0;
    long resp_pkts = 0;
    long resp_ip_bytes = 0;
};

class RastreadorFlujos {
public:
    explicit RastreadorFlujos(double edad_maxima_segundos = 300.0,
                               std::chrono::seconds intervalo_limpieza = std::chrono::seconds(60),
                               double ventana_maxima_segundos = 5.0);
    ~RastreadorFlujos();

    RastreadorFlujos(const RastreadorFlujos&) = delete;
    RastreadorFlujos& operator=(const RastreadorFlujos&) = delete;

    ResultadoFlujo actualizar_flujo(const std::string& ip_origen, int puerto_origen,
                                     const std::string& ip_destino, int puerto_destino,
                                     int protocolo, int longitud_paquete, double instante_actual);

    size_t flujos_rastreados() const;

private:
    void ciclo_limpieza();
    void limpiar_una_vez(double instante_actual);

    mutable std::mutex mutex_mapa_;
    std::unordered_map<ClaveFlujo, EstadoFlujo, HashClaveFlujo> flujos_;
    double inicio_;
    double edad_maxima_segundos_;
    double ventana_maxima_segundos_;
    std::chrono::seconds intervalo_limpieza_;

    std::mutex mutex_variable_condicion_;
    std::condition_variable variable_condicion_;
    std::atomic<bool> en_ejecucion_{true};
    std::thread hilo_limpieza_;
};

}