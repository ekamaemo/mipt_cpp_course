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

std::size_t CheckRules(const Event& event, const Rule* rules, std::size_t rule_count) {
    std::size_t count = 0;
    for (std::size_t i = 0; i < rule_count; ++i) {
        const Rule& rule = rules[i];
        if (rule.check(event)) {
            std::print("[DETECT] {}  {}  ts={} pid={}\n",
                       SeverityName(rule.severity),
                       rule.id,
                       event.raw_ts(),
                       event.pid());
            ++count;
        }
    }
    return count;
}

}  // namespace nano_edr