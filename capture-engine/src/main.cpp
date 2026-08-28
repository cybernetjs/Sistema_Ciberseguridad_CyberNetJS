#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>

#include "event_batcher.h"
#include "feature_extractor.h"
#include "json_sender.h"
#include "logger.h"
#include "packet_capture.h"
#include "shutdown_signal.h"

namespace {

constexpr int kDefaultPort = 9999;
constexpr size_t kBatchSize = 25;
constexpr auto kFlushInterval = std::chrono::milliseconds(1000);

}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr,
                      "Uso: %s <ip_laptop> [puerto=%d]\n"
                      "Captura y envio de trafico real desde la Raspberry Pi al NIDS central\n",
                      argv[0], kDefaultPort);
        return 1;
    }

    std::string laptop_ip = argv[1];
    int port = (argc >= 3) ? std::atoi(argv[2]) : kDefaultPort;
    std::string bpf_filter = "ip and not port " + std::to_string(port);

    nids::ShutdownSignal::instance().arm();

    nids::PacketCapture capture(bpf_filter);
    std::string cap_err;
    if (!capture.open(&cap_err)) {
        nids::Logger::instance().error("No se pudo abrir la captura: " + cap_err);
        nids::Logger::instance().error("Sugerencia: ejecuta este programa con sudo (se requiere acceso raw a la red).");
        return 1;
    }

    nids::JsonLineSender sender(laptop_ip, port);
    nids::EventBatcher batcher(sender, kBatchSize, kFlushInterval);
    nids::FlowTracker tracker;
    int link_type = capture.datalink();

    nids::Logger::instance().info("Capturando trafico real en raspberry -> " + laptop_ip + ":" + std::to_string(port));
    nids::Logger::instance().info("Interfaz: any (todas) | filtro BPF: " + bpf_filter + " | batch: " + std::to_string(kBatchSize));
    nids::Logger::instance().info("Tipo de enlace detectado (DLT): " + std::to_string(link_type));

    batcher.start_auto_flush();

    std::thread capture_thread([&]() {
        capture.loop([&](const uint8_t* pkt, uint32_t caplen) {
            auto ev = nids::extract_event(pkt, caplen, tracker, link_type);
            if (ev) {
                batcher.add_event(std::move(*ev));
            }
        });
    });

    while (nids::ShutdownSignal::instance().is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    nids::Logger::instance().info("Deteniendo captura...");
    capture.break_loop();
    if (capture_thread.joinable()) {
        capture_thread.join();
    }
    batcher.stop_auto_flush();
    batcher.flush(true);
    sender.close();

    nids::Logger::instance().info(
        "Eventos capturados: " + std::to_string(batcher.captured_total()) +
        " | Eventos enviados: " + std::to_string(batcher.sent_total()) +
        " | destino: " + laptop_ip + ":" + std::to_string(port));

    return 0;
}

