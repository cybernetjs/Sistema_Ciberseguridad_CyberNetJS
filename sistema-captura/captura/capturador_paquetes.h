#pragma once

#include <cstdint>
#include <functional>
#include <string>

struct pcap;
typedef struct pcap pcap_t;
struct pcap_pkthdr;

namespace sdi {

class CapturadorPaquetes {
public:
    using ManejadorPaquete = std::function<void(const uint8_t* paquete, uint32_t longitud_capturada)>;

    CapturadorPaquetes(std::string interfaz, std::string filtro_bpf);
    ~CapturadorPaquetes();

    bool abrir(std::string* mensaje_error);

    int tipo_enlace() const;

    void escuchar(const ManejadorPaquete& manejador);

    void detener();

private:
    static void redirigir_captura_pcap(uint8_t* usuario,
                                        const pcap_pkthdr* cabecera,
                                        const uint8_t* datos);

    std::string interfaz_;
    std::string filtro_;
    pcap_t* manejador_pcap_ = nullptr;
};

}