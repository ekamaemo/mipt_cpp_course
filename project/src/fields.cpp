#include<fields.h>
#include<stdexcept>
#include<cstdint>
#include<cctype>
namespace nano_edr{

namespace{
    std::string LowerString(const std::string& str){
        std::string res;
        res.reserve(str.size());
        for (char c: str){
            res.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
        return res;
    }

    void ReplacePart(std::string& str, const std::string& from, const std::string& to){
        size_t start_pos;
        start_pos = str.find(from);
        if (start_pos != std::string::npos){
            str.replace(start_pos, from.size(), to);
        }
    }
}

const std::string* FindField(const Event& event, const std::string& key){
    for (const Field& field: event.fields){
        if (field.key == key){
            return &(field.value);
        }
    }
    return nullptr;
}

const std::string& GetRequiredField(const Event& event, const std::string& key){
    const std::string* val = FindField(event, key);
    if (val == nullptr){
        throw std::invalid_argument("required field missing: " + key);
    }
    return *val;
}

bool GetIntField(const Event& event, const std::string& key, uint64_t* out){
    const std::string* value = FindField(event, key);
    if (value == nullptr || value->empty()){
        return false;
    }

    if (value->find_first_not_of("0123456789") != std::string::npos){
        return false;
    }
    
    try {
        unsigned long long parsed = std::stoull(*value);
        *out = static_cast<uint64_t>(parsed);
        return true;
    } catch (const std::exception&) {
        // переполнение 
        return false;
    }

}

uint64_t GetIntField(const Event& event, const std::string& key, uint64_t fallback){
    uint64_t value = 0;
    if (GetIntField(event, key, &value)){
        return value;
    }
    return fallback;
}


// предикаты
bool IsProcessStart(const Event& event){
    return event.type == "process_start";
}

bool IsFileWrite(const Event& event){
    return event.type == "file_write";
}

bool IsNetConnect(const Event& event){
    return event.type == "net_connect";
}

std::string NormalizePath(const std::string& path){
    std::string path_new;
    path_new.reserve(path.size());
    for (char c: LowerString(path)){
        if (c == '/' || c == '\\'){
            if (path_new.empty() || path_new.back() != '\\'){
                path_new.push_back('\\');
            }
        } else {
            path_new.push_back(c);
        }
    }
    
    ReplacePart(path_new, "%temp%", "\\appdata\\local\\temp");
    ReplacePart(path_new, "%tmp%", "\\appdata\\local\\temp");
    return path_new;
}

bool PathEndsWith(const Event& event, const std::string& suffix){
    const std::string* path = FindField(event, "path");
    if (path == nullptr){
        return false;
    }

    std::string normal_path = NormalizePath(*path);
    std::string normal_suffix = NormalizePath(suffix);
    if (normal_path.ends_with(normal_suffix)){
        return true;
    }

    return false;
}

bool CommandLineContains(const Event& event, const std::string& needle){
    const std::string* cmdline = FindField(event, "cmdline");
    if (cmdline == nullptr){
        return false;
    }
    std::string normal_cmdline = NormalizePath(*cmdline);
    return normal_cmdline.find(NormalizePath(needle)) != std::string::npos;
}

}
