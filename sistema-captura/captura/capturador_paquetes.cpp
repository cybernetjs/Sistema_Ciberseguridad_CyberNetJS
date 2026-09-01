#include "capturador_paquetes.h"

#include <pcap/pcap.h>

namespace sdi {

CapturadorPaquetes::CapturadorPaquetes(std::string interfaz, std::string filtro_bpf)
    : interfaz_(std::move(interfaz)), filtro_(std::move(filtro_bpf)) {}

CapturadorPaquetes::~CapturadorPaquetes() {
    if (manejador_pcap_) {
        pcap_close(manejador_pcap_);
        manejador_pcap_ = nullptr;
    }
}

bool CapturadorPaquetes::abrir(std::string* mensaje_error) {
    char buffer_error[PCAP_ERRBUF_SIZE] = {0};

    manejador_pcap_ = pcap_create(interfaz_.c_str(), buffer_error);
    if (!manejador_pcap_) {
        if (mensaje_error) *mensaje_error = buffer_error;
        return false;
    }

    pcap_set_snaplen(manejador_pcap_, 65535);
    pcap_set_promisc(manejador_pcap_, 1);
    pcap_set_timeout(manejador_pcap_, 1000);
    pcap_set_immediate_mode(manejador_pcap_, 1);

    if (pcap_activate(manejador_pcap_) < 0) {
        if (mensaje_error) *mensaje_error = pcap_geterr(manejador_pcap_);
        pcap_close(manejador_pcap_);
        manejador_pcap_ = nullptr;
        return false;
    }

    struct bpf_program programa{};
    if (pcap_compile(manejador_pcap_, &programa, filtro_.c_str(), 1, PCAP_NETMASK_UNKNOWN) != 0) {
        if (mensaje_error) *mensaje_error = pcap_geterr(manejador_pcap_);
        return false;
    }
    if (pcap_setfilter(manejador_pcap_, &programa) != 0) {
        if (mensaje_error) *mensaje_error = pcap_geterr(manejador_pcap_);
        pcap_freecode(&programa);
        return false;
    }
    pcap_freecode(&programa);

    return true;
}

int CapturadorPaquetes::tipo_enlace() const {
    return manejador_pcap_ ? pcap_datalink(manejador_pcap_) : -1;
}

void CapturadorPaquetes::redirigir_captura_pcap(uint8_t* usuario,
                                                 const pcap_pkthdr* cabecera,
                                                 const uint8_t* datos) {
    auto* manejador = reinterpret_cast<const ManejadorPaquete*>(usuario);
    (*manejador)(datos, cabecera->caplen);
}

void CapturadorPaquetes::escuchar(const ManejadorPaquete& manejador) {
    if (!manejador_pcap_) return;

    pcap_loop(manejador_pcap_, -1, &CapturadorPaquetes::redirigir_captura_pcap,
              reinterpret_cast<uint8_t*>(const_cast<ManejadorPaquete*>(&manejador)));
}

void CapturadorPaquetes::detener() {
    if (manejador_pcap_) {
        pcap_breakloop(manejador_pcap_);
    }
}

}