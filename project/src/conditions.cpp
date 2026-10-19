#include "conditions.h"
#include "text.h"
#include "fields.h"

namespace nano_edr {
bool FieldEquals::Matches(const Event& event) const{
    const std::string* v = FindField(event, key_);
    if (v == nullptr) {
        return false;
    }
    return EqualsIgnoreCase(*v, value_);
}


bool FieldContains::Matches(const Event& event) const{
    const std::string* v = FindField(event, key_);
    if (v == nullptr){
        return false;
    }
    return ContainsIgnoreCase(*v, needle_);
}

bool FieldEndsWith::Matches(const Event& event) const {
    const std::string* v = FindField(event, key_);
    if (v == nullptr) {
        return false;
    }
    return EndsWithIgnoreCase(*v, suffix_);
}


bool FieldInList::Matches(const Event& event) const{
    const std::string* v = FindField(event, key_);
    if (v == nullptr) {
        return false;
    }
    for (const std::string& value: values_){
        if (EqualsIgnoreCase(*v, value)){
            return true;
        }
    }
    return false;
}

bool EventTypeIs::Matches(const Event& event) const{
    for (const EventType& type: types_){
        if (event.event_type() == type){
            return true;
        }
    }
    return false;
}

void AllOf::Add(std::unique_ptr<ICondition> condition){
    parts_.push_back(std::move(condition));
}

void AnyOf::Add(std::unique_ptr<ICondition> condition) {
    parts_.push_back(std::move(condition));
}

bool AllOf::Matches(const Event& event) const {
    for (size_t i = 0; i < size(); ++i){
        if (!parts_[i]->Matches(event)){
            return false;
        }
    }
    return true;
}

bool AnyOf::Matches(const Event& event) const {
    for (size_t i = 0; i < size(); ++i){
        if (parts_[i]->Matches(event)){
            return true;
        }
    }
    return false;
}
}