#pragma once
#include "agent.h"

namespace nano_edr {
class OsSource {
 public:
    explicit OsSource(const std::string& config_path)
        : handle_(config_path) {}

    void Run(Agent* agent);

 private:
    OsHandle handle_;
};
} 