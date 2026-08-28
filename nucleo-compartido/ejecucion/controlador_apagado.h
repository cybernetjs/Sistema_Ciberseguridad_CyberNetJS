#pragma once

#include <atomic>

namespace sdi {

class ControladorApagado {
public:
    static ControladorApagado& instancia();

    void activar();
    bool debe_continuar() const;

    ControladorApagado(const ControladorApagado&) = delete;
    ControladorApagado& operator=(const ControladorApagado&) = delete;

private:
    ControladorApagado() = default;
    static void atender_senal(int numero_senal);

    std::atomic<bool> en_ejecucion_{true};
};

}
