#include "shutdown_signal.h"

#include <csignal>

namespace nids {

ShutdownSignal& ShutdownSignal::instance() {
    static ShutdownSignal instance;
    return instance;
}

void ShutdownSignal::arm() {
    std::signal(SIGINT, &ShutdownSignal::handle_signal);
    std::signal(SIGTERM, &ShutdownSignal::handle_signal);
}

bool ShutdownSignal::is_running() const { return running_.load(); }

void ShutdownSignal::handle_signal(int) { instance().running_.store(false); }

}

