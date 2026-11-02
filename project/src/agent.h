#pragma once
#include <event_list.h>
#include <unordered_map>
#include "rule.h"
#include "agent_rules.h"
#include "event_source.h"

namespace nano_edr {

class Agent : public IEventHandler{
 public:
    Agent(std::size_t window_size, bool quiet) : window_(window_size), engine_(BuildAgentRules()), quiet_(quiet), total_(0), comments_(0){};  // из --window-size и --quiet
    void HandleEvent(const Event& event) override;
    void OnSkipped() override;
    void OnBroken() override;
    void OnTick() override;

    void PrintSummary() const;
    void CountComment();
    void CountMalformed();
    void CountRejected();

 private:
    EventList window_;
    RuleEngine engine_;
    bool quiet_ = false;
    std::size_t total_ = 0;
    std::size_t comments_ = 0;
    size_t skipped_ = 0;
    std::size_t broken_ = 0;
    const Event* prev_prev_event_ = nullptr;
    std::unordered_map<std::string, unsigned> types_;
    void PrintContext() const;
};
}  // namespace nano_edr