cmake_minimum_required(VERSION 3.16)
project(nids_capture_engine LANGUAGES CXX)

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

add_executable(nids_capture
    src/main.cpp
    src/feature_extractor.cpp
    src/packet_capture.cpp
    src/json_sender.cpp
    src/event_batcher.cpp
    src/logger.cpp
    src/shutdown_signal.cpp
)

target_include_directories(nids_capture PRIVATE
    ${PCAP_INCLUDE_DIR}
    src
    third_party
)
target_link_libraries(nids_capture PRIVATE ${PCAP_LIBRARY} pthread)

message(STATUS "libpcap include: ${PCAP_INCLUDE_DIR}")
message(STATUS "libpcap library: ${PCAP_LIBRARY}")
