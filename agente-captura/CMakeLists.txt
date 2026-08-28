cmake_minimum_required(VERSION 3.16)
project(agente_captura LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_path(PCAP_INCLUDE_DIR NAMES pcap/pcap.h pcap.h)
find_library(PCAP_LIBRARY NAMES pcap)

if(NOT PCAP_INCLUDE_DIR OR NOT PCAP_LIBRARY)
    message(FATAL_ERROR "libpcap no encontrado. Instala con: sudo apt install libpcap-dev")
endif()

set(RAIZ_PROYECTO ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(agente_captura
    punto_entrada/principal.cpp
    dominio/rastreador_flujos.cpp
    captura/capturador_paquetes.cpp
    extraccion/extractor_caracteristicas.cpp
    transporte/serializador_json.cpp
    transporte/transmisor_json.cpp
    aplicacion/loteador_eventos.cpp
    ${RAIZ_PROYECTO}/nucleo-compartido/diagnostico/bitacora.cpp
    ${RAIZ_PROYECTO}/nucleo-compartido/ejecucion/controlador_apagado.cpp
)

target_include_directories(agente_captura PRIVATE
    ${PCAP_INCLUDE_DIR}
    dominio
    captura
    extraccion
    transporte
    aplicacion
    ${RAIZ_PROYECTO}/dependencias-externas
    ${RAIZ_PROYECTO}/nucleo-compartido/dominio
    ${RAIZ_PROYECTO}/nucleo-compartido/diagnostico
    ${RAIZ_PROYECTO}/nucleo-compartido/ejecucion
    ${RAIZ_PROYECTO}/nucleo-compartido/red
    ${RAIZ_PROYECTO}/nucleo-compartido/tiempo
)

target_link_libraries(agente_captura PRIVATE ${PCAP_LIBRARY} pthread)

message(STATUS "libpcap include: ${PCAP_INCLUDE_DIR}")
message(STATUS "libpcap library: ${PCAP_LIBRARY}")
