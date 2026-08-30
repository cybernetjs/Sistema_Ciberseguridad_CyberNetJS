#pragma once

#include <string>

namespace sdi {

// Intenta lanzar panel-control (PanelControl.exe, la interfaz grafica WinUI 3)
// como proceso hijo, pasandole por variables de entorno las mismas rutas que
// esta usando servicio_inferencia (CSV de eventos, metricas del modelo y log),
// de modo que al ejecutar unicamente servicio_inferencia.exe ya aparezca la
// interfaz grafica sin pasos adicionales.
//
// Busca PanelControl.exe en varias rutas candidatas relativas a la ubicacion
// del propio ejecutable (ver lanzador_panel_control.cpp). Si no lo encuentra,
// o si la plataforma no es Windows, no hace nada y deja constancia en
// 'motivo' de por que no se lanzo.
//
// Devuelve true si el proceso se lanzo correctamente.
bool lanzar_panel_control(const std::string& ruta_csv, const std::string& ruta_metricas,
                           const std::string& ruta_log, std::string* motivo_si_fallo);

// Calcula, a partir de la ruta al modelo de arboles (el tercer argumento de
// servicio_inferencia.exe), una ruta razonable para el metricas.json que
// espera panel-control. Se puede ignorar pasando explicitamente el quinto
// argumento del ejecutable.
std::string derivar_ruta_metricas(const std::string& ruta_modelo);

}
