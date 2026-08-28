#include "json_sender.h"

#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include <nlohmann/json.hpp>

#include "shutdown_signal.h"

namespace nids {

using json = nlohmann::json;

namespace {

constexpr int kConnectTimeoutMs = 2000;
constexpr int kConnectPollStepMs = 200;
constexpr int kIoTimeoutSec = 10;
constexpr int kMaxSendAttempts = 3;
constexpr int kRetryDelayMs = 250;

}

json event_to_json_object(const NetworkEvent& ev) {
    return json{
        {"src_ip", ev.src_ip},
        {"dst_ip", ev.dst_ip},
        {"dst_port", ev.dst_port},
        {"protocol", ev.protocol},
        {"dur", ev.dur},
        {"spkts", ev.spkts},
        {"dpkts", ev.dpkts},
        {"sbytes", ev.sbytes},
        {"dbytes", ev.dbytes},
        {"rate", ev.rate},
        {"sttl", ev.sttl},
        {"dttl", ev.dttl},
        {"sload", ev.sload},
        {"dload", ev.dload},
        {"sinpkt", ev.sinpkt},
        {"dinpkt", ev.dinpkt},
        {"sjit", ev.sjit},
        {"djit", ev.djit},
        {"ct_srv_src", ev.ct_srv_src},
        {"ct_dst_ltm", ev.ct_dst_ltm},
    };
}

std::string event_to_json(const NetworkEvent& ev) {
    return event_to_json_object(ev).dump();
}

std::string batch_to_json_payload(const std::vector<NetworkEvent>& batch) {
    if (batch.size() == 1) {
        return event_to_json_object(batch.front()).dump();
    }
    json arr = json::array();
    arr.get_ref<json::array_t&>().reserve(batch.size());
    for (const auto& ev : batch) {
        arr.push_back(event_to_json_object(ev));
    }
    return arr.dump();
}

JsonLineSender::JsonLineSender(std::string host, int port)
    : host_(std::move(host)), port_(port) {}

JsonLineSender::~JsonLineSender() { close(); }

bool JsonLineSender::ensure_connected() {
    if (sock_fd_ != -1) {
        return true;
    }

    struct addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* res = nullptr;
    std::string port_str = std::to_string(port_);
    if (getaddrinfo(host_.c_str(), port_str.c_str(), &hints, &res) != 0) {
        return false;
    }

    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        freeaddrinfo(res);
        return false;
    }

    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    int rc = connect(fd, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    bool connected = false;

    if (rc == 0) {
        connected = true;
    } else if (errno == EINPROGRESS) {
        int waited_ms = 0;
        while (waited_ms < kConnectTimeoutMs) {
            if (!ShutdownSignal::instance().is_running() && waited_ms > 0) {
                break;
            }
            struct pollfd pfd{};
            pfd.fd = fd;
            pfd.events = POLLOUT;
            int pr = poll(&pfd, 1, kConnectPollStepMs);
            if (pr > 0 && (pfd.revents & (POLLOUT | POLLERR | POLLHUP))) {
                int so_error = 0;
                socklen_t len = sizeof(so_error);
                getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &len);
                connected = (so_error == 0);
                break;
            }
            waited_ms += kConnectPollStepMs;
        }
    }

    if (!connected) {
        ::close(fd);
        return false;
    }

    fcntl(fd, F_SETFL, flags);

    struct timeval tv{};
    tv.tv_sec = kIoTimeoutSec;
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    sock_fd_ = fd;
    return true;
}

void JsonLineSender::close() {
    if (sock_fd_ != -1) {
        ::close(sock_fd_);
        sock_fd_ = -1;
    }
}

bool JsonLineSender::send(const std::vector<NetworkEvent>& batch) {
    if (batch.empty()) {
        return true;
    }

    std::string line = batch_to_json_payload(batch) + "\n";

    for (int attempt = 0; attempt < kMaxSendAttempts; ++attempt) {
        if (!ensure_connected()) {
            close();
            std::this_thread::sleep_for(std::chrono::milliseconds(kRetryDelayMs));
            continue;
        }

        size_t total_sent = 0;
        const char* data = line.data();
        size_t len = line.size();
        bool ok = true;

        while (total_sent < len) {
            ssize_t n = ::send(sock_fd_, data + total_sent, len - total_sent, 0);
            if (n <= 0) {
                ok = false;
                break;
            }
            total_sent += static_cast<size_t>(n);
        }

        if (ok) {
            return true;
        }

        close();
        std::this_thread::sleep_for(std::chrono::milliseconds(kRetryDelayMs));
    }

    return false;
}

}

