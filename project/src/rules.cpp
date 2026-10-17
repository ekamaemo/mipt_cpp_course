#include <rules.h>
#include <print>


namespace nano_edr {

const char* SeverityName(Severity severity) {
    switch (severity) {
        case Severity::kCritical: return "critical";
        case Severity::kHigh: return "high";
        case Severity::kLow: return "low";
        case Severity::kMedium: return "medium";
    }
    return "?";
}

}  // namespace nano_edr