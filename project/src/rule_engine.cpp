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
                fired->push_back(rules_[i].get());
            }
        }
    }
    return count;   // не забудь вернуть
}
}