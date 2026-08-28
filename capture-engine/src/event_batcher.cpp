#include "event_batcher.h"

#include <sstream>

#include "logger.h"

namespace nids {

EventBatcher::EventBatcher(ISender& sender, size_t batch_size, std::chrono::milliseconds flush_interval)
    : sender_(sender),
      batch_size_(batch_size),
      flush_interval_(flush_interval),
      started_(std::chrono::steady_clock::now()) {
    batch_.reserve(batch_size_);
}

EventBatcher::~EventBatcher() { stop_auto_flush(); }

void EventBatcher::add_event(NetworkEvent event) {
    captured_total_++;
    bool should_flush = false;
    {
        std::lock_guard<std::mutex> lock(batch_mutex_);
        batch_.push_back(std::move(event));
        should_flush = batch_.size() >= batch_size_;
    }
    if (should_flush) {
        flush(false);
    }
}

void EventBatcher::start_auto_flush() {
    auto_flush_running_ = true;
    flush_thread_ = std::thread(&EventBatcher::auto_flush_loop, this);
}

void EventBatcher::stop_auto_flush() {
    auto_flush_running_ = false;
    if (flush_thread_.joinable()) {
        flush_thread_.join();
    }
}

void EventBatcher::auto_flush_loop() {
    while (auto_flush_running_) {
        std::this_thread::sleep_for(flush_interval_);
        flush(false);
    }
}

void EventBatcher::flush(bool force) {
    std::vector<NetworkEvent> to_send;
    {
        std::lock_guard<std::mutex> lock(batch_mutex_);
        if (batch_.empty()) return;
        if (!force && batch_.size() < batch_size_) return;
        to_send.swap(batch_);
        batch_.reserve(batch_size_);
    }

    bool ok = sender_.send(to_send);

    if (ok) {
        if (!connected_once_.exchange(true)) {
            Logger::instance().info("Conexion establecida con el NIDS central.");
        }
        sent_total_ += to_send.size();
        size_t nb = ++batches_sent_;

        if (warn_connection_.exchange(0) > 0) {
            Logger::instance().info("Conexion restablecida con el NIDS central.");
        }

        if (nb % kStatusLogEvery == 0) {
            double elapsed = std::chrono::duration<double>(
                                  std::chrono::steady_clock::now() - started_)
                                  .count();
            elapsed = std::max(1.0, elapsed);
            double eps = sent_total_ / elapsed;

            std::ostringstream msg;
            msg << "Enviando trafico OK | eventos enviados: " << sent_total_.load()
                << " | tasa aprox: " << eps << " ev/s";
            Logger::instance().info(msg.str());
        }
        return;
    }

    int wc = ++warn_connection_;
    if (wc == 1 || wc % 10 == 0) {
        Logger::instance().warn(
            "No se pudo enviar al NIDS central. Reintentando... "
            "verifica que el servicio remoto este activo y el puerto accesible.");
    }

    std::lock_guard<std::mutex> lock(batch_mutex_);
    if (to_send.size() > kMaxBacklog) {
        to_send.erase(to_send.begin(), to_send.end() - kMaxBacklog);
    }
    for (auto& ev : to_send) {
        batch_.push_back(std::move(ev));
    }
}

size_t EventBatcher::sent_total() const { return sent_total_.load(); }
size_t EventBatcher::captured_total() const { return captured_total_.load(); }

}

