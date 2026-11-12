#include "agent.h"
#include <print>

#include "agent_rules.h"
#include "rules.h"

namespace nano_edr {
Agent::Agent(bool quiet) : engine_(BuildAgentRules()), quiet_(quiet){
    engine_.SetModel(&model_);
};

void Agent::HandleEvent(const Event& event) {
    model_.Observe(event);
    contextPrinted_ = false;
    engine_.ProcessEvent(event, nullptr);
    ++total_;
    ++types_[event.type()];
    window_.PushBack(WindowEntry{event.ts(), event.type(), event.pid()});
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
    std::print("[STAT] в окне занято {} из {} мест\n", window_.size(), window_.capacity());
    std::print("[STAT] в модели процессов {}, файлов {}\n", model_.process_count(), model_.file_count());
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
    if (window_.size() >= 2) {
        std::print(
            "[CTX] {}: {} {} pid={}\n",
            -2,
            window_[window_.size() - 2].ts.ms,
            window_[window_.size() - 2].type,
            window_[window_.size() - 2].pid);
    }

    if (!window_.empty()) {
        std::print(
            "[CTX] {}: {} {} pid={}\n",
            -1,
            window_.back().ts.ms,
            window_.back().type,
            window_.back().pid);
    }
}

void Agent::Subscribe(AlertSink sink) {
    FanOutSink all;
    all.Add([this](const Detection&) { PrintContextOnce(); });
    all.Add(std::move(sink));
    engine_.Subscribe(std::move(all));
}

void Agent::PrintContextOnce() {
    if (!contextPrinted_ && !quiet_) {
        PrintContext();
        contextPrinted_ = true;
    }
}
}  // namespace nano_edr
