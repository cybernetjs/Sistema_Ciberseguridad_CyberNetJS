#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

#include "evento_red.h"
#include "interfaz_transmisor_eventos.h"

namespace sdi {

class LoteadorEventos {
public:
    LoteadorEventos(ITransmisorEventos& transmisor, size_t tamano_lote, std::chrono::milliseconds intervalo_envio);
    ~LoteadorEventos();

    void agregar_evento(EventoRed evento);

    void iniciar_envio_automatico();
    void detener_envio_automatico();

    void vaciar(bool forzar);

    size_t total_enviado() const;
    size_t total_capturado() const;

private:
    void ciclo_envio_automatico();

    ITransmisorEventos& transmisor_;
    size_t tamano_lote_;
    std::chrono::milliseconds intervalo_envio_;

    std::mutex mutex_lote_;
    std::vector<EventoRed> lote_;

    std::atomic<bool> envio_automatico_activo_{false};
    std::thread hilo_envio_;

    std::atomic<size_t> total_enviado_{0};
    std::atomic<size_t> total_capturado_{0};
    std::atomic<size_t> lotes_enviados_{0};
    std::atomic<int> advertencias_conexion_{0};
    std::atomic<bool> conectado_alguna_vez_{false};
    std::chrono::steady_clock::time_point inicio_;

    static constexpr size_t RESPALDO_MAXIMO = 5000;
    static constexpr size_t FRECUENCIA_REPORTE_ESTADO = 20;
};

}
