#include "loteador_eventos.h"

#include <algorithm>
#include <sstream>

#include "bitacora.h"

namespace sdi {

LoteadorEventos::LoteadorEventos(ITransmisorEventos& transmisor, size_t tamano_lote,
                                  std::chrono::milliseconds intervalo_envio)
    : transmisor_(transmisor),
      tamano_lote_(tamano_lote),
      intervalo_envio_(intervalo_envio),
      inicio_(std::chrono::steady_clock::now()) {
    lote_.reserve(tamano_lote_);
}

LoteadorEventos::~LoteadorEventos() { detener_envio_automatico(); }

void LoteadorEventos::agregar_evento(EventoRed evento) {
    total_capturado_++;
    bool debe_vaciar = false;
    {
        std::lock_guard<std::mutex> bloqueo(mutex_lote_);
        lote_.push_back(std::move(evento));
        debe_vaciar = lote_.size() >= tamano_lote_;
    }
    if (debe_vaciar) {
        vaciar(false);
    }
}

void LoteadorEventos::iniciar_envio_automatico() {
    envio_automatico_activo_ = true;
    hilo_envio_ = std::thread(&LoteadorEventos::ciclo_envio_automatico, this);
}

void LoteadorEventos::detener_envio_automatico() {
    envio_automatico_activo_ = false;
    if (hilo_envio_.joinable()) {
        hilo_envio_.join();
    }
}

void LoteadorEventos::ciclo_envio_automatico() {
    while (envio_automatico_activo_) {
        std::this_thread::sleep_for(intervalo_envio_);
        vaciar(false);
    }
}

void LoteadorEventos::vaciar(bool forzar) {
    std::vector<EventoRed> a_enviar;
    {
        std::lock_guard<std::mutex> bloqueo(mutex_lote_);
        if (lote_.empty()) return;
        if (!forzar && lote_.size() < tamano_lote_) return;
        a_enviar.swap(lote_);
        lote_.reserve(tamano_lote_);
    }

    bool exito = transmisor_.enviar(a_enviar);

    if (exito) {
        if (!conectado_alguna_vez_.exchange(true)) {
            Bitacora::instancia().registrar_info("Conexion establecida con el servicio de inferencia.");
        }
        total_enviado_ += a_enviar.size();
        size_t numero_lote = ++lotes_enviados_;

        if (advertencias_conexion_.exchange(0) > 0) {
            Bitacora::instancia().registrar_info("Conexion restablecida con el servicio de inferencia.");
        }

        if (numero_lote % FRECUENCIA_REPORTE_ESTADO == 0) {
            double transcurrido = std::chrono::duration<double>(
                                       std::chrono::steady_clock::now() - inicio_)
                                       .count();
            transcurrido = std::max(1.0, transcurrido);
            double eventos_por_segundo = total_enviado_ / transcurrido;

            std::ostringstream mensaje;
            mensaje << "Enviando trafico OK | eventos enviados: " << total_enviado_.load()
                    << " | tasa aproximada: " << eventos_por_segundo << " ev/s";
            Bitacora::instancia().registrar_info(mensaje.str());
        }
        return;
    }

    int contador_advertencias = ++advertencias_conexion_;
    if (contador_advertencias == 1 || contador_advertencias % 10 == 0) {
        Bitacora::instancia().registrar_advertencia(
            "No se pudo enviar al servicio de inferencia. Reintentando... "
            "verifica que el servicio remoto este activo y el puerto accesible.");
    }

    std::lock_guard<std::mutex> bloqueo(mutex_lote_);
    if (a_enviar.size() > RESPALDO_MAXIMO) {
        a_enviar.erase(a_enviar.begin(), a_enviar.end() - RESPALDO_MAXIMO);
    }
    for (auto& evento : a_enviar) {
        lote_.push_back(std::move(evento));
    }
}

size_t LoteadorEventos::total_enviado() const { return total_enviado_.load(); }
size_t LoteadorEventos::total_capturado() const { return total_capturado_.load(); }

}
