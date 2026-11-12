#include "detection.h"

namespace nano_edr {
std::string ToString(const Detection& detection){
    std::string result = "[DETECT] " +
                         std::string(SeverityName(detection.severity)) +
                         " " + detection.rule +
                         " ts=" + std::to_string(detection.ts.ms) + 
                         " pid=" + detection.pid;

    if (!detection.chain.empty()) {
        result += " " + detection.chain;
    }
    return result;
}
}