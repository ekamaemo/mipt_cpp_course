#include "text.h"

namespace nano_edr {


std::string ToLower(const std::string& str) {
    std::string res;
    res.reserve(str.size());
    for (char c : str) {
        res.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return res;
}

bool EqualsIgnoreCase(const std::string& line_a, const std::string& line_b){
    return ToLower(line_a) == ToLower(line_b);

}

bool ContainsIgnoreCase(const std::string& line, const std::string& needle){
    return ToLower(line).find(ToLower(needle)) != std::string::npos;
}


bool EndsWithIgnoreCase(const std::string& line, const std::string& suffix){
    return ToLower(line).ends_with(ToLower(suffix)); 
}
}