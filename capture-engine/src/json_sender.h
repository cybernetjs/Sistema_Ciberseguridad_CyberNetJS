#pragma once

#include <string>
#include <vector>

#include "feature_extractor.h"
#include "i_sender.h"

namespace nids {

std::string event_to_json(const NetworkEvent& ev);
std::string batch_to_json_payload(const std::vector<NetworkEvent>& batch);

class JsonLineSender : public ISender {
public:
    JsonLineSender(std::string host, int port);
    ~JsonLineSender() override;

    bool send(const std::vector<NetworkEvent>& batch) override;
    void close() override;

private:
    bool ensure_connected();

    std::string host_;
    int port_;
    int sock_fd_ = -1;
};

}

