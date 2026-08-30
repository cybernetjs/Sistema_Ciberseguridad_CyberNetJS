#pragma once

#include <fstream>
#include <mutex>
#include <ostream>
#include <string>

namespace sdi {

class Bitacora {
public:
    static Bitacora& instancia();

    void abrir_archivo(const std::string& ruta_archivo);

    void registrar_info(const std::string& mensaje);
    void registrar_advertencia(const std::string& mensaje);
    void registrar_error(const std::string& mensaje);

    Bitacora(const Bitacora&) = delete;
    Bitacora& operator=(const Bitacora&) = delete;

private:
    Bitacora() = default;
    void escribir(std::ostream& salida, const std::string& nivel, const std::string& mensaje);

    std::mutex mutex_;
    std::ofstream archivo_;
};

}
