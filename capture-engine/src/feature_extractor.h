#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>

namespace nids {

struct NetworkEvent {
    std::string src_ip;
    std::string dst_ip;
    int dst_port = 1;
    int protocol = 0;
    double dur = 0.001;
    int spkts = 1;
    int dpkts = 0;
    int sbytes = 0;
    int dbytes = 0;
    double rate = 0.0;
    int sttl = 64;
    int dttl = 0;
    double sload = 0.0;
    double dload = 0.0;
    double sinpkt = 0.0;
    double dinpkt = 0.0;
    double sjit = 0.0;
    double djit = 0.0;
    int ct_srv_src = 1;
    int ct_dst_ltm = 1;
};

struct FlowKey {
    std::string src_ip;
    std::string dst_ip;
    int dst_port = 0;
    int protocol = 0;

    bool operator==(const FlowKey& other) const {
        return dst_port == other.dst_port &&
               protocol == other.protocol &&
               src_ip == other.src_ip &&
               dst_ip == other.dst_ip;
    }
};

struct FlowKeyHash {
    size_t operator()(const FlowKey& k) const {
        size_t h1 = std::hash<std::string>{}(k.src_ip);
        size_t h2 = std::hash<std::string>{}(k.dst_ip);
        size_t h3 = std::hash<int>{}(k.dst_port);
        size_t h4 = std::hash<int>{}(k.protocol);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
    }
};

class FlowTracker {
public:
    explicit FlowTracker(double max_age_seconds = 300.0,
                          std::chrono::seconds cleanup_interval = std::chrono::seconds(60));
    ~FlowTracker();

    FlowTracker(const FlowTracker&) = delete;
    FlowTracker& operator=(const FlowTracker&) = delete;

    double update_and_get_duration(const FlowKey& key, double now);

    size_t tracked_flows() const;

    static double now_seconds();

private:
    void cleanup_loop();
    void cleanup_once(double now);

    mutable std::mutex map_mutex_;
    std::unordered_map<FlowKey, double, FlowKeyHash> last_seen_;
    double started_;
    double max_age_seconds_;
    std::chrono::seconds cleanup_interval_;

    std::mutex cv_mutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_{true};
    std::thread cleanup_thread_;
};

std::optional<NetworkEvent> extract_event(const uint8_t* packet,
                                           uint32_t caplen,
                                           FlowTracker& tracker,
                                           int link_type);

}

