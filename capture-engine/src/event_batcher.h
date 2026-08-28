#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

#include "feature_extractor.h"
#include "i_sender.h"

namespace nids {

class EventBatcher {
public:
    EventBatcher(ISender& sender, size_t batch_size, std::chrono::milliseconds flush_interval);
    ~EventBatcher();

    void add_event(NetworkEvent event);

    void start_auto_flush();
    void stop_auto_flush();

    void flush(bool force);

    size_t sent_total() const;
    size_t captured_total() const;

private:
    void auto_flush_loop();

    ISender& sender_;
    size_t batch_size_;
    std::chrono::milliseconds flush_interval_;

    std::mutex batch_mutex_;
    std::vector<NetworkEvent> batch_;

    std::atomic<bool> auto_flush_running_{false};
    std::thread flush_thread_;

    std::atomic<size_t> sent_total_{0};
    std::atomic<size_t> captured_total_{0};
    std::atomic<size_t> batches_sent_{0};
    std::atomic<int> warn_connection_{0};
    std::atomic<bool> connected_once_{false};
    std::chrono::steady_clock::time_point started_;

    static constexpr size_t kMaxBacklog = 5000;
    static constexpr size_t kStatusLogEvery = 20;
};

}

