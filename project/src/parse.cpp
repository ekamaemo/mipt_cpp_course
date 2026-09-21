#include<../kit/include/l1.2/parse.h>

namespace nano_edr {
    bool IsBlankOrComment(const std::string* line) {
        std::size_t i = 0;
        while (i < line->size() && ((*line)[i] == ' ' || (*line)[i] == '\t')){
            i++;
        }
        if (i == line->size()){
            return true;
        } 
        if ((*line)[i] == ';' || (*line)[i] == '#'){
            return true;
        }
        return false;
    }

    bool ParseFields(const std::string* text, std::vector<Field>* out){
        std::size_t i = 0;
        std::size_t k_start = 0;
        std::string key;
        std::size_t v_start = 0;
        while (i < text->size()){
            while (i < text->size() && ((*text)[i] == ' ' || (*text)[i] == '\t')){
                i++;
            }
            if (i == text->size()){
                return true;
            }
            k_start = i;
            while (i <text->size() && (*text)[i] != '=' && (*text)[i] != ' ' && (*text)[i] != '\t'){
                i++;
            }
            if (i == text->size() || (*text)[i] != '='){
                return false;
            }
            if (i == k_start){
                return false;
            }

            key = text->substr(k_start, i-k_start);
            i++;
            
            std::string value;
            // если есть двойные кавычки
            if (i < text->size() && (*text)[i] == '"'){
                i++;
                while (i < text->size() && (*text)[i] != '"'){
                    value += (*text)[i];
                    i++;
                }
                if (i == text->size() ||(*text)[i] != '"'){
                    return false;
                }
                i++;
                if (i < text->size() && (*text)[i] != ' ' && (*text)[i]!= '\t'){
                    return false;
                }
            } else {
                // нет кавычек
                v_start = i;
                while (i < text->size() && (*text)[i] != ' ' && (*text)[i] != '\t'){
                    i++;
                }
                value = text->substr(v_start, i - v_start);
            }
        out->push_back({key, value});
        }
    return true;
    }

    bool ParseEventLine(const std::string* line, Event* out){
        if (IsBlankOrComment(line)){
            return false;
        }
        std::vector<Field> fields;
        if (!ParseFields(line, &fields)){
            return false;
        }

        bool flag_ts = false;
        bool flag_type = false;
        for (const Field& field: fields){
            if (field.key == "ts"){
                flag_ts = true;
                out->ts = field.value;
            } else if (field.key == "type"){
                flag_type = true;
                out->type = field.value;
            } else if (field.key == "pid"){
                out->pid = field.value;
            } else {
                out->fields.push_back(field);
            }
        }
        if (flag_ts && flag_type){
            return true;
        }
        return false;
    }
}