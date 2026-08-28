#pragma once

#include <cstdint>
#include <functional>
#include <string>

struct pcap;
typedef struct pcap pcap_t;
struct pcap_pkthdr;

namespace nids {

class PacketCapture {
public:
    using PacketHandler = std::function<void(const uint8_t* packet, uint32_t caplen)>;

    explicit PacketCapture(std::string bpf_filter);
    ~PacketCapture();

    bool open(std::string* error_out);

    int datalink() const;

    void loop(const PacketHandler& handler);

    void break_loop();

private:
    static void pcap_callback_trampoline(uint8_t* user,
                                          const pcap_pkthdr* header,
                                          const uint8_t* bytes);

    std::string filter_;
    pcap_t* handle_ = nullptr;
};

}

