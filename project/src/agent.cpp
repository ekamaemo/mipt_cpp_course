#include "agent.h"

#include <iostream>
#include <print>

#include "agent_rules.h"
#include "event_list.h"
#include "rules.h"

namespace nano_edr {
    
EventParts ConvertOsEvent(const os_event* ev) {
    EventParts parts;
    parts.ts = std::to_string(ev->ts);
    parts.type = ev->type != nullptr ? ev->type : "";
    parts.pid = (ev->pid != 0) ? std::to_string(ev->pid) : "";

    for (std::size_t i = 0; i < ev->field_count; ++i) {
        const os_kv& kv = ev->fields[i];
        parts.fields.push_back(Field{
            kv.key != nullptr ? kv.key : "",
            kv.value != nullptr ? kv.value : ""});
    }
    return parts;
}

void Agent::HandleEvent(const Event& event) {
    std::size_t count = 0;
    for (std::size_t i = 0; i < rules_.size(); ++i){
        if (rules_[i].Check(event)){
            std::print("[DETECT] {}  {}  ts={} pid={}\n",
                       SeverityName(rules_[i].severity()),
                       rules_[i].id(),
                       event.raw_ts(),
                       event.pid());
            ++count;
        }
    }
    ++total_;
    ++types_[event.type()];
    if (count != 0 && !quiet_){
        PrintContext();
    }
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
    std::print("Битых строк: {}, отвергнутых событий: {}\n", malformed_, rejected_);
    for (const auto& [type, count] : types_) {
        std::print(" {}: {}\n", type, count);
    }

    std::print("правила, детектов:\n");
    for (MatchRule rule: rules_){
        std::print(" {}  {}\n", rule.id(), rule.hits());
    }
}

void Agent::CountComment() {
    ++comments_;
}

void Agent::CountRejected() {
    ++rejected_;
}

void Agent::CountMalformed() {
    ++malformed_;
}


void Agent::Trampoline(const os_event* ev, void* ctx) noexcept {
    Agent* agent = static_cast<Agent*>(ctx);
    try {
        EventParts parts = ConvertOsEvent(ev);
        Event event(parts);
        agent->HandleEvent(event);
    } catch (const std::exception& error) {
        agent->CountRejected();
        std::print(stderr, "Error reason: {}\n", error.what());
    } catch (...) {
        agent->CountRejected();
        std::print(stderr, "Unknown error reason\n");
    }
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
