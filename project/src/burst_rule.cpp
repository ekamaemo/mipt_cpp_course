#include "burst_rule.h"

namespace nano_edr {
BurstRule::BurstRule(const std::string& id, Severity severity, EventType type, uint64_t window_ms) : RuleBase(id, severity), type_(type), window_ms_(window_ms){};

bool BurstRule::Check(const Event& event){
    const uint64_t now_ms = event.ts().ms;
    for (auto it = pending_.begin(); it != pending_.end(); ) {
        if (now_ms - it->ts_ms > window_ms_) {
            it = pending_.erase(it);
        } else {
            ++it;
        }
    }
    if (event.event_type() != type_){
        return false;
    }
    std::size_t found = pending_.size();   
    for (std::size_t i = 0; i < pending_.size(); ++i) {
        if (pending_[i].pid == event.pid()) {
                found = i;
                break;
        }
    }
    if (found != pending_.size()) {
        pending_[found].ts_ms = event.ts().ms;
        CountHit();
        return true;
    }
    pending_.push_back(Pending(event.ts().ms, event.type()));
    return false;
}
}