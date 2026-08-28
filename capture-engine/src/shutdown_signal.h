#pragma once

#include <atomic>

namespace nids {

class ShutdownSignal {
public:
    static ShutdownSignal& instance();

    void arm();
    bool is_running() const;

    ShutdownSignal(const ShutdownSignal&) = delete;
    ShutdownSignal& operator=(const ShutdownSignal&) = delete;

private:
    ShutdownSignal() = default;
    static void handle_signal(int signum);

    std::atomic<bool> running_{true};
};

}

