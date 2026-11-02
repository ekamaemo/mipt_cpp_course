#include "rule.h"
#include <cstddef>

namespace nano_edr {

RuleBase::RuleBase(const std::string& id, Severity severity) : id_(id), severity_(severity) {}

void RuleBase::AddAction(const std::string& action){
    actions_.push_back(action);
}

MatchRule::MatchRule(const std::string& id, Severity severity) : RuleBase(id, severity) {}

void MatchRule::AddCondition(std::unique_ptr<ICondition> condition){
    conditions_.push_back(std::move(condition));
}

bool MatchRule::Check(const Event& event){
    for (std::size_t i = 0; i < condition_count(); ++i) {
        if (!conditions_[i]->Matches(event)){
            return false;
        }
    }
    CountHit();
    return true;
}

SequenceRule::SequenceRule(const std::string& id, Severity severity,
                           uint64_t window_ms)
    : RuleBase(id, severity), window_ms_(window_ms) {}

void SequenceRule::SetFirst(std::unique_ptr<ICondition> condition) {
    first_ = std::move(condition);
}

void SequenceRule::SetSecond(std::unique_ptr<ICondition> condition) {
    second_= std::move(condition);
}

bool SequenceRule::Check(const Event& event) {
    if (!first_ || !second_){
        return false;
    } 
    const uint64_t now_ms = event.ts().ms;
    for (auto it = pending_.begin(); it != pending_.end(); ) {
        if (now_ms - it->ts_ms > window_ms_) {
            it = pending_.erase(it);
        } else {
            ++it;
        }
    }

    if (second_->Matches(event)) {
        std::size_t found = pending_.size();   
        for (std::size_t i = 0; i < pending_.size(); ++i) {
            if (!same_pid_ || pending_[i].pid == event.pid()) {
                found = i;
                break;
            }
        }
        if (found != pending_.size()) {
            pending_.erase(pending_.begin() + static_cast<std::ptrdiff_t>(found));
            CountHit();
            return true;
        }
    }

    if (first_->Matches(event)) {
        pending_.push_back(Pending{now_ms, event.pid()});
    }
    return false;
}
}
