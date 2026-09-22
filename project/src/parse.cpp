#include <parse.h>

namespace nano_edr {
    namespace {

        bool isSpace(const char element) {
            return element == ' ' || element == '\t';
        }
    }

    bool IsBlankOrComment(const std::string* line) {
        std::size_t i = line->find_first_not_of(" \t");

        if (i == std::string::npos) {
            return true;
        }

        if ((*line)[i] == ';' || (*line)[i] == '#') {
            return true;
        }
        return false;
    }

    bool ParseEventLine(const std::string* line, Event* out) {
        if (IsBlankOrComment(line)) {
            return false;
        }

        bool flag_ts = false;
        bool flag_type = false;

        std::size_t i = 0;
        while (i < line->size()) {
            while (i < line->size() && isSpace((*line)[i])) {
                ++i;
            }
            if (i == line->size()) {
                break;
            }

            std::size_t k_start = i;
            while (i < line->size() && (*line)[i] != '=' && !isSpace((*line)[i])) {
                ++i;
            }

            if (i == line->size() || (*line)[i] != '=') {
                return false;
            }

            if (i == k_start) {
                return false;
            }

            std::string key = line->substr(k_start, i - k_start);
            ++i;

           
            std::string value;
            if (i < line->size() && (*line)[i] == '"') {
                ++i;
                while (i < line->size() && (*line)[i] != '"') {
                    value += (*line)[i];
                    ++i;
                }
                if (i == line->size() || (*line)[i] != '"') {
                    return false;
                }
                ++i;
                if (i < line->size() && !isSpace((*line)[i])) {
                    return false;
                }
            } else {
                std::size_t v_start = i;
                while (i < line->size() && !isSpace((*line)[i])) {
                    ++i;
                }
                value = line->substr(v_start, i - v_start);
            }

            
            if (key == "ts" && !flag_ts) {
                flag_ts = true;
                out->ts = value;
            } else if (key == "type" && !flag_type) {
                flag_type = true;
                out->type = value;
            } else if (key == "pid") {
                out->pid = value;
            } else {
                out->fields.push_back({key, value});
            }
        }

        return flag_ts && flag_type;
    }

}