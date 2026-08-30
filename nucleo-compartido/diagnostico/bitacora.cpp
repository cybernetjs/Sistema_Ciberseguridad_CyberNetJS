#include "bitacora.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

namespace sdi {

Bitacora& Bitacora::instancia() {
    static Bitacora unica;
    return unica;
}

void Bitacora::abrir_archivo(const std::string& ruta_archivo) {
    std::lock_guard<std::mutex> bloqueo(mutex_);
    archivo_.open(ruta_archivo, std::ios::app);
}

void Bitacora::escribir(std::ostream& salida, const std::string& nivel, const std::string& mensaje) {
    std::lock_guard<std::mutex> bloqueo(mutex_);
    auto ahora = std::chrono::system_clock::now();
    std::time_t ahora_c = std::chrono::system_clock::to_time_t(ahora);
    std::tm marca_tiempo{};
#ifdef _WIN32
    localtime_s(&marca_tiempo, &ahora_c);
#else
    localtime_r(&ahora_c, &marca_tiempo);
#endif

    salida << "[" << std::put_time(&marca_tiempo, "%H:%M:%S") << "] [" << nivel << "] " << mensaje << std::endl;

    if (archivo_.is_open()) {
        archivo_ << "[" << std::put_time(&marca_tiempo, "%H:%M:%S") << "] [" << nivel << "] " << mensaje << std::endl;
        archivo_.flush();
    }
}

void Bitacora::registrar_info(const std::string& mensaje) { escribir(std::cout, "INFO", mensaje); }
void Bitacora::registrar_advertencia(const std::string& mensaje) { escribir(std::cout, "ADVERTENCIA", mensaje); }
void Bitacora::registrar_error(const std::string& mensaje) { escribir(std::cerr, "ERROR", mensaje); }

}
