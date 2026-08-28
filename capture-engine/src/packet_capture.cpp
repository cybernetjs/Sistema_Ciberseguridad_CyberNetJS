#include "packet_capture.h"

#include <pcap/pcap.h>

#include <cstring>
#include <iostream>

namespace nids {

PacketCapture::PacketCapture(std::string bpf_filter) : filter_(std::move(bpf_filter)) {}

PacketCapture::~PacketCapture() {
    if (handle_) {
        pcap_close(handle_);
        handle_ = nullptr;
    }
}

bool PacketCapture::open(std::string* error_out) {
    char errbuf[PCAP_ERRBUF_SIZE] = {0};

    handle_ = pcap_create("any", errbuf);
    if (!handle_) {
        if (error_out) *error_out = errbuf;
        return false;
    }

    pcap_set_snaplen(handle_, 65535);
    pcap_set_promisc(handle_, 1);
    pcap_set_timeout(handle_, 1000);
    pcap_set_immediate_mode(handle_, 1);

    if (pcap_activate(handle_) < 0) {
        if (error_out) *error_out = pcap_geterr(handle_);
        pcap_close(handle_);
        handle_ = nullptr;
        return false;
    }

    struct bpf_program prog{};
    if (pcap_compile(handle_, &prog, filter_.c_str(), 1, PCAP_NETMASK_UNKNOWN) != 0) {
        if (error_out) *error_out = pcap_geterr(handle_);
        return false;
    }
    if (pcap_setfilter(handle_, &prog) != 0) {
        if (error_out) *error_out = pcap_geterr(handle_);
        pcap_freecode(&prog);
        return false;
    }
    pcap_freecode(&prog);

    return true;
}

int PacketCapture::datalink() const {
    return handle_ ? pcap_datalink(handle_) : -1;
}

void PacketCapture::pcap_callback_trampoline(uint8_t* user,
                                              const pcap_pkthdr* header,
                                              const uint8_t* bytes) {
    auto* handler = reinterpret_cast<const PacketHandler*>(user);
    (*handler)(bytes, header->caplen);
}

void PacketCapture::loop(const PacketHandler& handler) {
    if (!handle_) return;

    pcap_loop(handle_, -1, &PacketCapture::pcap_callback_trampoline,
              reinterpret_cast<uint8_t*>(const_cast<PacketHandler*>(&handler)));
}

void PacketCapture::break_loop() {
    if (handle_) {
        pcap_breakloop(handle_);
    }
}

}

