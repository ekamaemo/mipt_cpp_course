#pragma once
#include <event_list.h>
#include <unordered_map>
#include "rule.h"
#include "agent_rules.h"
#include "event_source.h"

namespace nano_edr {
struct WindowEntry {
    Timestamp ts;
    std::string type;
    std::string pid;
};


class Agent : public IEventHandler{
 public:
    static constexpr std::size_t kWindowCapacity = 256;
    explicit Agent(bool quiet);
    void HandleEvent(const Event& event) override;
    void OnSkipped() override;
    void OnBroken() override;
    void OnTick() override;

    void PrintSummary() const;
    void CountComment();
    void CountMalformed();
    void CountRejected();

    void Subscribe(AlertSink sink);

 private:
    RingBuffer<WindowEntry, kWindowCapacity> window_;
    EntityModel model_;
    RuleEngine engine_;
    bool quiet_ = false;
    std::size_t total_ = 0;
    std::size_t comments_ = 0;
    size_t skipped_ = 0;
    std::size_t broken_ = 0;
    std::unordered_map<std::string, unsigned> types_;
    void PrintContext() const;
    void PrintContextOnce();
    bool contextPrinted_ = false;
};
}  // namespace nano_edr