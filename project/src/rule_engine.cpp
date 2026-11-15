#include "rule_engine.h"

namespace nano_edr {
void RuleEngine::AddRule(std::unique_ptr<IRule> rule){
    rules_.push_back(std::move(rule));
}

std::size_t RuleEngine::ProcessEvent(const Event& event, std::vector<const IRule*>* fired){
    std::size_t count = 0;
    for (std::size_t i = 0; i < size(); ++i){
        if (rules_[i]->Check(event)){
            ++count;
            if (fired != nullptr){
                fired->push_back(&rule(i));
            }
            Detection detection = MakeDetection(rule(i), event);
            for (const AlertSink& sink : sinks_){
                sink(detection);
            }
        }
    }
    return count;   
}

void RuleEngine::Subscribe(AlertSink sink) {
    sinks_.push_back(std::move(sink));
}

Detection RuleEngine::MakeDetection(const IRule& rule, const Event& event) const {
    Detection detection;
    detection.rule = rule.id();
    detection.severity = rule.severity();
    detection.ts = event.ts();
    detection.pid = event.pid();
    if (model_ != nullptr) {
        detection.chain = model_->DescribeChain(event.pid());
    }
    detection.actions = rule.actions();
    return detection;
}
}