#include<rules.h>
#include<iostream>

namespace nano_edr {

const char* SeverityName(Severity severity){
    switch (severity) {
        case Severity::kCritical : return "critical";
        case Severity::kHigh : return "high";
        case Severity::kLow : return "low";
        case Severity::kMedium : return "medium";
    }
    return "?";
}

size_t CheckRules(const Event& event, const Rule* rules, size_t rule_count){
    size_t count = 0;
    for (size_t i = 0; i < rule_count; ++i){
        const Rule& rule = rules[i];
        if (rule.check(event)) {
            std::cout << "[DETECT] " << SeverityName(rule.severity)
                      << "  " << rule.id
                      << "  ts=" << event.ts
                      << " pid=" << event.pid
                      << "\n";
            ++count;
        }
    }

    return count;
}

}