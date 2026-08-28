#include "feature_extractor.h"

#include <algorithm>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>

namespace nids {

namespace {

uint16_t read_be16(const uint8_t* p) {
    return static_cast<uint16_t>((p[0] << 8) | p[1]);
}

constexpr int kDltEthernet = 1;
constexpr int kDltLinuxSll = 113;
constexpr int kDltLinuxSll2 = 276;
constexpr int kDltRaw = 12;

struct LinkLayerInfo {
    int header_len;
    int ethertype_offset;
    bool has_ethertype;
    bool supported;
};

LinkLayerInfo link_layer_info(int link_type) {
    switch (link_type) {
        case kDltEthernet:
            return {14, 12, true, true};
        case kDltLinuxSll:
            return {16, 14, true, true};
        case kDltLinuxSll2:
            return {20, 0, true, true};
        case kDltRaw:
            return {0, 0, false, true};
        default:
            return {0, 0, false, false};
    }
}

constexpr uint16_t kEthertypeIp = 0x0800;

}

FlowTracker::FlowTracker(double max_age_seconds, std::chrono::seconds cleanup_interval)
    : started_(now_seconds()),
      max_age_seconds_(max_age_seconds),
      cleanup_interval_(cleanup_interval) {
    cleanup_thread_ = std::thread(&FlowTracker::cleanup_loop, this);
}

FlowTracker::~FlowTracker() {
    {
        std::lock_guard<std::mutex> lock(cv_mutex_);
        running_ = false;
    }
    cv_.notify_all();
    if (cleanup_thread_.joinable()) {
        cleanup_thread_.join();
    }
}

double FlowTracker::now_seconds() {
    using namespace std::chrono;
    return duration<double>(system_clock::now().time_since_epoch()).count();
}

double FlowTracker::update_and_get_duration(const FlowKey& key, double now) {
    std::lock_guard<std::mutex> lock(map_mutex_);
    auto it = last_seen_.find(key);
    double last = (it != last_seen_.end()) ? it->second : started_;
    double dur = std::max(0.001, now - last);
    last_seen_[key] = now;
    return dur;
}

size_t FlowTracker::tracked_flows() const {
    std::lock_guard<std::mutex> lock(map_mutex_);
    return last_seen_.size();
}

void FlowTracker::cleanup_once(double now) {
    std::lock_guard<std::mutex> lock(map_mutex_);
    for (auto it = last_seen_.begin(); it != last_seen_.end();) {
        if (now - it->second > max_age_seconds_) {
            it = last_seen_.erase(it);
        } else {
            ++it;
        }
    }
}

void FlowTracker::cleanup_loop() {
    std::unique_lock<std::mutex> lock(cv_mutex_);
    while (running_) {
        bool stopped = cv_.wait_for(lock, cleanup_interval_, [this] { return !running_.load(); });
        if (stopped) {
            break;
        }
        cleanup_once(now_seconds());
    }
}

std::optional<NetworkEvent> extract_event(const uint8_t* packet,
                                           uint32_t caplen,
                                           FlowTracker& tracker,
                                           int link_type) {
    LinkLayerInfo info = link_layer_info(link_type);
    if (!info.supported) {
        return std::nullopt;
    }

    if (caplen < static_cast<uint32_t>(info.header_len)) {
        return std::nullopt;
    }

    if (info.has_ethertype) {
        uint16_t ethertype = read_be16(packet + info.ethertype_offset);
        if (ethertype != kEthertypeIp) {
            return std::nullopt;
        }
    }

    const uint8_t* ip_start = packet + info.header_len;
    uint32_t ip_avail = caplen - info.header_len;
    if (ip_avail < sizeof(struct ip)) {
        return std::nullopt;
    }

    const auto* iph = reinterpret_cast<const struct ip*>(ip_start);
    int ip_header_len = iph->ip_hl * 4;
    if (ip_header_len < 20 || static_cast<uint32_t>(ip_header_len) > ip_avail) {
        return std::nullopt;
    }

    char src_buf[INET_ADDRSTRLEN] = {0};
    char dst_buf[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &iph->ip_src, src_buf, sizeof(src_buf));
    inet_ntop(AF_INET, &iph->ip_dst, dst_buf, sizeof(dst_buf));

    if (src_buf[0] == '\0' || dst_buf[0] == '\0') {
        return std::nullopt;
    }

    int protocol = iph->ip_p;

    const uint8_t* l4_start = ip_start + ip_header_len;
    uint32_t l4_avail = ip_avail - ip_header_len;

    int dst_port = 1;
    if ((protocol == IPPROTO_TCP || protocol == IPPROTO_UDP) && l4_avail >= 4) {
        dst_port = read_be16(l4_start + 2);
    }

    double now = FlowTracker::now_seconds();
    FlowKey key{src_buf, dst_buf, dst_port, protocol};
    double dur = tracker.update_and_get_duration(key, now);

    int pkt_len = static_cast<int>(caplen);
    int sttl = iph->ip_ttl;

    NetworkEvent ev;
    ev.src_ip = src_buf;
    ev.dst_ip = dst_buf;
    ev.dst_port = dst_port;
    ev.protocol = protocol;
    ev.dur = dur;
    ev.spkts = 1;
    ev.dpkts = 0;
    ev.sbytes = pkt_len;
    ev.dbytes = 0;
    ev.rate = 1.0 / dur;
    ev.sttl = sttl;
    ev.dttl = 0;
    ev.sload = pkt_len / dur;
    ev.dload = 0.0;
    ev.sinpkt = dur;
    ev.dinpkt = dur;
    ev.sjit = 0.0;
    ev.djit = 0.0;
    ev.ct_srv_src = 1;
    ev.ct_dst_ltm = 1;

    return ev;
}

}

