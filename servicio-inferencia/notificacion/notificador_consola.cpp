#include "notificador_consola.h"

#include <sstream>

#include "bitacora.h"

namespace sdi {

void NotificadorConsola::notificar(const EventoRed& evento, const VeredictoClasificacion& veredicto,
                                    double tiempo_respuesta_ms) {
    std::ostringstream mensaje;
    mensaje << "ALERTA | " << veredicto.etiqueta
            << " | origen=" << evento.ip_origen
            << " destino=" << evento.ip_destino << ":" << evento.puerto_destino
            << " confianza=" << veredicto.confianza
            << " tiempo_respuesta_ms=" << tiempo_respuesta_ms;
    Bitacora::instancia().registrar_advertencia(mensaje.str());
}

}
