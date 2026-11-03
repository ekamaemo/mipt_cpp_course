#include "rule.h"
// Срабатывает, когда процесс присылает событие типа type не позже чем
// через window_ms после своего предыдущего события этого типа.
namespace nano_edr {
class BurstRule : public RuleBase {
 public:
    BurstRule(const std::string& id, Severity severity, EventType type,
              uint64_t window_ms);

    bool Check(const Event& event) override;

 private:
     struct Pending {
        uint64_t ts_ms{};
        std::string pid;
    };

    EventType type_;
    uint64_t window_ms_;
    std::vector<Pending> pending_;
};
}
