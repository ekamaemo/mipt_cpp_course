#include "agent.h"
#include <print>

#include "agent_rules.h"
#include "event_list.h"
#include "rules.h"

namespace nano_edr {

void Agent::HandleEvent(const Event& event) {
    std::vector<const IRule*> fired;
    engine_.ProcessEvent(event, &fired);
    bool contextPrinted = false;
    for (std::size_t i = 0; i < fired.size(); ++i){
        if (!contextPrinted && !quiet_){
            PrintContext();
            contextPrinted = true;
        }
        std::print("[DETECT] {}  {}  ts={} pid={}\n",
                   SeverityName(fired[i]->severity()),
                       fired[i]->id(),
                       event.raw_ts(),
                       event.pid());
    }
    ++total_;
    ++types_[event.type()];
    if (!window_.empty()) {
        prev_prev_event_ = &window_.back()->event;
    }
    window_.PushBack(event);
}

void Agent::PrintSummary() const {
    if (quiet_) {
        return;
    }
    std::print("Всего событий: {}, всего комментариев: {}\n", total_, comments_);
    std::print("Битых строк: {}, отвергнутых событий: {}\n", skipped_, broken_);
    for (const auto& [type, count] : types_) {
        std::print(" {}: {}\n", type, count);
    }

    std::print("правила, детектов:\n");
    for (std::size_t i = 0; i < engine_.size(); ++i){
        std::print(" {}  {}\n", engine_.rule(i).id(), engine_.rule(i).hits());
    }
}

void Agent::CountComment() {
    ++comments_;
}


void Agent::OnSkipped() {
    ++skipped_;
}

void Agent::OnBroken() {
    ++broken_;
}

void Agent::OnTick() {
    // пусто
}


void Agent::PrintContext() const{
    if (prev_prev_event_ != nullptr) {
        std::print(
            "[CTX] {}: ts={} type={} pid={}\n",
            -2,
            prev_prev_event_->raw_ts(),
            prev_prev_event_->type(),
            prev_prev_event_->pid());
    }

    if (!window_.empty()) {
        std::print(
            "[CTX] {}: ts={} type={} pid={}\n",
            -1,
            window_.back()->event.raw_ts(),
            window_.back()->event.type(),
            window_.back()->event.pid());
    }
}
}  // namespace nano_edr
