#include "detection.h"

namespace nano_edr {
std::string ToString(const Detection& detection){
    std::string result = "[DETECT] " +
                         std::to_string(static_cast<int>(detection.severity)) +
                         " " + detection.rule +
                         " ts=" + std::to_string(detection.ts.ms) + 
                         " pid=" + detection.pid;

    if (!detection.chain.empty()) {
        result += " " + detection.chain;
    }
    return result;
}
}