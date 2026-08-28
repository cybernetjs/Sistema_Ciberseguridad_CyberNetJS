#pragma once

#include <chrono>

namespace sdi::tiempo {

inline double segundos_actuales() {
    using namespace std::chrono;
    return duration<double>(system_clock::now().time_since_epoch()).count();
}

}
