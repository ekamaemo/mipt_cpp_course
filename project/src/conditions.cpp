#include "conditions.h"
#include "text.h"
#include "fields.h"

namespace nano_edr {
bool FieldEquals::operator()(const Event& event) const{
    const std::string* v = FindField(event, key_);
    if (v == nullptr) {
        return false;
    }
    return EqualsIgnoreCase(*v, value_);
}


bool FieldContains::operator()(const Event& event) const{
    const std::string* v = FindField(event, key_);
    if (v == nullptr){
        return false;
    }
    return ContainsIgnoreCase(*v, needle_);
}

bool FieldEndsWith::operator()(const Event& event) const {
    const std::string* v = FindField(event, key_);
    if (v == nullptr) {
        return false;
    }
    return EndsWithIgnoreCase(*v, suffix_);
}


bool FieldInList::operator()(const Event& event) const{
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


Condition Condition::Equals(const std::string& key, const std::vector<std::string>& values){
    return Condition(Match::kEquals, key, values);
}

Condition Condition::Contains(const std::string& key, const std::vector<std::string>& values){
    return Condition(Match::kContains, key, values);
}

Condition Condition::EndsWith(const std::string& key, const std::vector<std::string>& values){
    return Condition(Match::kEndsWith, key, values);
}

bool Condition::operator()(const Event& event) const {
    const std::string *v = FindField(event, key_);
    if (v == nullptr){
        return false;
    }
    switch (match_){
        case Match::kEquals : {
            for (const std::string& value: values_){
                if (EqualsIgnoreCase(*v, value)){
                    return true;
                }
            }
            return false;
        }

        case Match::kContains : {
            for (const std::string& value: values_){
                if (ContainsIgnoreCase(*v, value)){
                    return true;
                }
            }
            return false;
        }

        case Match::kEndsWith : {
            for (const std::string& value: values_){
                if (EndsWithIgnoreCase(*v, value)){
                    return true;
                }
            }
            return false;
        }
    }
}
}