#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>

#include "bitacora.h"
#include "capturador_paquetes.h"
#include "controlador_apagado.h"
#include "extractor_caracteristicas.h"
#include "loteador_eventos.h"
#include "rastreador_flujos.h"
#include "transmisor_json.h"

namespace {

constexpr int PUERTO_POR_DEFECTO = 9999;
constexpr size_t TAMANO_LOTE = 25;
constexpr auto INTERVALO_ENVIO = std::chrono::milliseconds(1000);
constexpr const char* INTERFAZ_POR_DEFECTO = "br0";

}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr,
                      "Uso: %s <ip_destino> [puerto=%d] [interfaz=%s]\n"
                      "Captura y envio de trafico real desde la Raspberry Pi al servicio de inferencia\n",
                      argv[0], PUERTO_POR_DEFECTO, INTERFAZ_POR_DEFECTO);
        return 1;
    }

    std::string ip_destino = argv[1];
    int puerto = (argc >= 3) ? std::atoi(argv[2]) : PUERTO_POR_DEFECTO;
    std::string interfaz = (argc >= 4) ? argv[3] : INTERFAZ_POR_DEFECTO;
    std::string filtro_bpf = "ip and not port " + std::to_string(puerto);

    sdi::ControladorApagado::instancia().activar();

    sdi::CapturadorPaquetes capturador(interfaz, filtro_bpf);
    std::string error_captura;
    if (!capturador.abrir(&error_captura)) {
        sdi::Bitacora::instancia().registrar_error("No se pudo abrir la captura: " + error_captura);
        sdi::Bitacora::instancia().registrar_error("Sugerencia: ejecuta este programa con privilegios elevados (se requiere acceso raw a la red).");
        return 1;
    }

    sdi::TransmisorJson transmisor(ip_destino, puerto);
    sdi::LoteadorEventos loteador(transmisor, TAMANO_LOTE, INTERVALO_ENVIO);
    sdi::RastreadorFlujos rastreador;
    int tipo_enlace = capturador.tipo_enlace();

    sdi::Bitacora::instancia().registrar_info("Capturando trafico real -> " + ip_destino + ":" + std::to_string(puerto));
    sdi::Bitacora::instancia().registrar_info("Interfaz: " + interfaz + " | filtro BPF: " + filtro_bpf + " | tamano de lote: " + std::to_string(TAMANO_LOTE));
    sdi::Bitacora::instancia().registrar_info("Tipo de enlace detectado: " + std::to_string(tipo_enlace));

    loteador.iniciar_envio_automatico();

    std::thread hilo_captura([&]() {
        capturador.escuchar([&](const uint8_t* paquete, uint32_t longitud_capturada) {
            auto evento = sdi::extraer_caracteristicas(paquete, longitud_capturada, rastreador, tipo_enlace);
            if (evento) {
                loteador.agregar_evento(std::move(*evento));
            }
        });
    });

    while (sdi::ControladorApagado::instancia().debe_continuar()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    sdi::Bitacora::instancia().registrar_info("Deteniendo captura...");
    capturador.detener();
    if (hilo_captura.joinable()) {
        hilo_captura.join();
    }
    loteador.detener_envio_automatico();
    loteador.vaciar(true);
    transmisor.cerrar();

    sdi::Bitacora::instancia().registrar_info(
        "Eventos capturados: " + std::to_string(loteador.total_capturado()) +
        " | Eventos enviados: " + std::to_string(loteador.total_enviado()) +
        " | destino: " + ip_destino + ":" + std::to_string(puerto));

    return 0;
}