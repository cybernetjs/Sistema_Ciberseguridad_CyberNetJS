#pragma once

#include <vector>

#include "feature_extractor.h"

namespace nids {

class ISender {
public:
    virtual ~ISender() = default;
    virtual bool send(const std::vector<NetworkEvent>& batch) = 0;
    virtual void close() = 0;
};

}

