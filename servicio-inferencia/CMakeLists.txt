cmake_minimum_required(VERSION 3.16)
project(servicio_inferencia LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(RAIZ_PROYECTO ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(servicio_inferencia
    punto_entrada/principal.cpp
    transporte/servidor_tcp.cpp
    transporte/decodificador_eventos_json.cpp
    deteccion/detector_firmas.cpp
    deteccion/detector_aprendizaje_automatico.cpp
    notificacion/notificador_consola.cpp
    aplicacion/canalizador_eventos.cpp
    ${RAIZ_PROYECTO}/nucleo-compartido/diagnostico/bitacora.cpp
    ${RAIZ_PROYECTO}/nucleo-compartido/ejecucion/controlador_apagado.cpp
)

target_include_directories(servicio_inferencia PRIVATE
    dominio
    deteccion
    notificacion
    transporte
    aplicacion
    ${RAIZ_PROYECTO}/dependencias-externas
    ${RAIZ_PROYECTO}/nucleo-compartido/dominio
    ${RAIZ_PROYECTO}/nucleo-compartido/diagnostico
    ${RAIZ_PROYECTO}/nucleo-compartido/ejecucion
    ${RAIZ_PROYECTO}/nucleo-compartido/red
    ${RAIZ_PROYECTO}/nucleo-compartido/tiempo
)

if(WIN32)
    target_link_libraries(servicio_inferencia PRIVATE ws2_32)
else()
    target_link_libraries(servicio_inferencia PRIVATE pthread)
endif()
