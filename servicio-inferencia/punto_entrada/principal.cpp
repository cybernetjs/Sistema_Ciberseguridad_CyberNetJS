#include <cstdio>
#include <cstdlib>
#include <string>

#include "bitacora.h"
#include "canalizador_eventos.h"
#include "compatibilidad_sockets.h"
#include "controlador_apagado.h"
#include "decodificador_eventos_json.h"
#include "detector_aprendizaje_automatico.h"
#include "detector_beaconing.h"
#include "detector_dga.h"
#include "detector_firmas.h"
#include "detector_fuerza_bruta.h"
#include "detector_reconocimiento.h"
#include "lanzador_panel_control.h"
#include "notificador_consola.h"
#include "registrador_csv.h"
#include "servidor_tcp.h"

namespace {

constexpr int PUERTO_POR_DEFECTO = 9999;
constexpr int UMBRAL_INUNDACION_PPS = 300;
constexpr double VENTANA_INUNDACION_SEGUNDOS = 1.0;
constexpr int UMBRAL_PUERTOS_ESCANEO = 15;
constexpr double VENTANA_ESCANEO_SEGUNDOS = 5.0;
constexpr int UMBRAL_INTENTOS_FUERZA_BRUTA = 8;
constexpr double VENTANA_FUERZA_BRUTA_SEGUNDOS = 300.0;
constexpr int UMBRAL_DOMINIOS_DGA = 6;
constexpr double VENTANA_DGA_SEGUNDOS = 30.0;
constexpr int MINIMO_REPETICIONES_BEACON = 10;
constexpr double INTERVALO_MIN_BEACON_SEGUNDOS = 20.0;
constexpr double INTERVALO_MAX_BEACON_SEGUNDOS = 180.0;
constexpr double VARIACION_MAXIMA_BEACON = 0.15;
constexpr size_t FRECUENCIA_REPORTE_ESTADO = 100;
constexpr const char* RUTA_CSV_POR_DEFECTO = "eventos_procesados.csv";
constexpr const char* RUTA_MODELO_POR_DEFECTO = "modelo_iot23_arboles.json";
constexpr const char* RUTA_LOG_POR_DEFECTO = "servicio.log";

}

int main(int argc, char** argv) {
    int puerto = (argc >= 2) ? std::atoi(argv[1]) : PUERTO_POR_DEFECTO;
    std::string ruta_csv = (argc >= 3) ? argv[2] : RUTA_CSV_POR_DEFECTO;
    std::string ruta_modelo = (argc >= 4) ? argv[3] : RUTA_MODELO_POR_DEFECTO;
    std::string ruta_log = (argc >= 5) ? argv[4] : RUTA_LOG_POR_DEFECTO;
    std::string ruta_metricas = (argc >= 6) ? argv[5] : sdi::derivar_ruta_metricas(ruta_modelo);

    sdi::Bitacora::instancia().abrir_archivo(ruta_log);

    std::string motivo_sin_panel;
    if (sdi::lanzar_panel_control(ruta_csv, ruta_metricas, ruta_log, &motivo_sin_panel)) {
        sdi::Bitacora::instancia().registrar_info("Interfaz grafica (panel-control) iniciada.");
    } else {
        sdi::Bitacora::instancia().registrar_advertencia(
            "No se inicio la interfaz grafica automaticamente: " + motivo_sin_panel);
    }

    if (!sdi::inicializar_sockets()) {
        sdi::Bitacora::instancia().registrar_error("No se pudo inicializar la capa de sockets del sistema.");
        return 1;
    }

    sdi::ControladorApagado::instancia().activar();

    sdi::ServidorTcp servidor(puerto);
    std::string error;
    if (!servidor.iniciar(&error)) {
        sdi::Bitacora::instancia().registrar_error("No se pudo iniciar el servidor: " + error);
        return 1;
    }

    sdi::DetectorReconocimiento detector_reconocimiento(UMBRAL_PUERTOS_ESCANEO, VENTANA_ESCANEO_SEGUNDOS);
    sdi::DetectorFuerzaBruta detector_fuerza_bruta(UMBRAL_INTENTOS_FUERZA_BRUTA, VENTANA_FUERZA_BRUTA_SEGUNDOS);
    sdi::DetectorDga detector_dga(UMBRAL_DOMINIOS_DGA, VENTANA_DGA_SEGUNDOS);
    sdi::DetectorBeaconing detector_beaconing(MINIMO_REPETICIONES_BEACON, INTERVALO_MIN_BEACON_SEGUNDOS,
                                               INTERVALO_MAX_BEACON_SEGUNDOS, VARIACION_MAXIMA_BEACON);
    sdi::DetectorFirmas detector_firmas(UMBRAL_INUNDACION_PPS, VENTANA_INUNDACION_SEGUNDOS);
    sdi::DetectorAprendizajeAutomatico detector_aprendizaje_automatico;

    bool modelo_cargado = detector_aprendizaje_automatico.cargar_modelo(ruta_modelo);
    if (modelo_cargado) {
        sdi::Bitacora::instancia().registrar_info("Modelo de aprendizaje automatico cargado: " + ruta_modelo);
    } else {
        sdi::Bitacora::instancia().registrar_error("No se pudo cargar el modelo de aprendizaje automatico: " + ruta_modelo);
    }

    sdi::NotificadorConsola notificador;
    sdi::RegistradorCsv registrador(ruta_csv);
    sdi::CanalizadorEventos canalizador(
        {&detector_reconocimiento, &detector_fuerza_bruta, &detector_dga, &detector_beaconing, &detector_firmas,
         &detector_aprendizaje_automatico},
        notificador, registrador);

    sdi::Bitacora::instancia().registrar_info("Servicio de inferencia escuchando en el puerto " + std::to_string(puerto));
    sdi::Bitacora::instancia().registrar_info(
        "Detectores activos: reconocimiento, fuerza_bruta, dga, beaconing, firmas (umbral flood " +
        std::to_string(UMBRAL_INUNDACION_PPS) + " pps), aprendizaje automatico (" +
        std::string(modelo_cargado ? "modelo cargado" : "sin modelo") + ")");

    servidor.ejecutar([&](const std::string& linea) {
        auto eventos = sdi::decodificar_eventos(linea);
        for (const auto& evento : eventos) {
            canalizador.procesar(evento);
        }
        if (canalizador.total_procesado() % FRECUENCIA_REPORTE_ESTADO == 0 && canalizador.total_procesado() > 0) {
            sdi::Bitacora::instancia().registrar_info(
                "Eventos procesados: " + std::to_string(canalizador.total_procesado()) +
                " | Alertas emitidas: " + std::to_string(canalizador.total_alertas()));
        }
    });

    servidor.detener();
    sdi::Bitacora::instancia().registrar_info(
        "Servidor detenido | Eventos procesados: " + std::to_string(canalizador.total_procesado()) +
        " | Alertas emitidas: " + std::to_string(canalizador.total_alertas()));

    sdi::finalizar_sockets();
    return 0;
}