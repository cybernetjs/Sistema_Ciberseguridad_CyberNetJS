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
    std::string ip_origen;
    std::string ip_destino;
    int puerto_destino = 0;
    int protocolo = 0;

    bool operator==(const ClaveFlujo& otra) const {
        return puerto_destino == otra.puerto_destino &&
               protocolo == otra.protocolo &&
               ip_origen == otra.ip_origen &&
               ip_destino == otra.ip_destino;
    }
};

struct HashClaveFlujo {
    size_t operator()(const ClaveFlujo& clave) const {
        size_t h1 = std::hash<std::string>{}(clave.ip_origen);
        size_t h2 = std::hash<std::string>{}(clave.ip_destino);
        size_t h3 = std::hash<int>{}(clave.puerto_destino);
        size_t h4 = std::hash<int>{}(clave.protocolo);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};

class RastreadorFlujos {
public:
    explicit RastreadorFlujos(double edad_maxima_segundos = 300.0,
                               std::chrono::seconds intervalo_limpieza = std::chrono::seconds(60));
    ~RastreadorFlujos();

    RastreadorFlujos(const RastreadorFlujos&) = delete;
    RastreadorFlujos& operator=(const RastreadorFlujos&) = delete;

    double actualizar_y_obtener_duracion(const ClaveFlujo& clave, double instante_actual);

    size_t flujos_rastreados() const;

private:
    void ciclo_limpieza();
    void limpiar_una_vez(double instante_actual);

    mutable std::mutex mutex_mapa_;
    std::unordered_map<ClaveFlujo, double, HashClaveFlujo> ultima_actividad_;
    double inicio_;
    double edad_maxima_segundos_;
    std::chrono::seconds intervalo_limpieza_;

    std::mutex mutex_variable_condicion_;
    std::condition_variable variable_condicion_;
    std::atomic<bool> en_ejecucion_{true};
    std::thread hilo_limpieza_;
};

}
