#pragma once
#include <event_list.h>

#include <unordered_map>

#include "agent_rules.h"
#include "event_source.h"
#include "rule.h"
#include "window_entry.h"
#include "response.h"

namespace nano_edr {

class Agent : public IEventHandler {
 public:
    static constexpr std::size_t kWindowCapacity = 256;
    explicit Agent(bool quiet, uint64_t dedup_period_ms);
    void HandleEvent(const Event& event) override;
    void OnSkipped() override;
    void OnBroken() override;
    void OnTick() override;

    void PrintSummary() const;
    void CountComment();
    void CountMalformed();
    void CountRejected();

    void Subscribe(AlertSink sink);
    void EnableResponse(OsHandle* os, bool dry_run,
                    const std::string& quarantine_dir);

    std::unique_ptr<ResponsePlanner> planner_{};
    std::unique_ptr<Responder> responder_{};


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
    bool context_printed_ = false;

    uint64_t dedup_period_ms_{};
    std::size_t suppressed_{};
};
}  // namespace nano_edr