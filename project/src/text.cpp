#include "text.h"

namespace nano_edr {
namespace {

// Понижение одного символа (безопасно для любых char).
inline unsigned char LowerAscii(unsigned char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? static_cast<unsigned char>(c + 32) : c;
}

// Сравнение двух символов без учёта регистра.
inline bool CharEqIgnoreCase(char a, char b) noexcept {
    return LowerAscii(static_cast<unsigned char>(a)) ==
           LowerAscii(static_cast<unsigned char>(b));
}
}

bool EqualsIgnoreCase(const std::string& line_a, const std::string& line_b) {
    if (line_a.size() != line_b.size()){ 
        return false;
    }
    for (std::size_t i = 0; i < line_a.size(); ++i) {
        if (!CharEqIgnoreCase(line_a[i], line_b[i])){
            return false;
        }
    }
    return true;
}

bool EndsWithIgnoreCase(const std::string& line, const std::string& suffix) {
    if (suffix.size() > line.size()){ 
        return false;
    }

    const std::size_t off = line.size() - suffix.size();
    for (std::size_t i = 0; i < suffix.size(); ++i) {
        if (!CharEqIgnoreCase(line[off + i], suffix[i])){
            return false;
        }
    }
    return true;
}

bool ContainsIgnoreCase(const std::string& line, const std::string& needle) {
    if (needle.empty()){
        return true;
    }
    if (needle.size() > line.size()){
        return false;
    }

    const std::size_t n = needle.size();
    const std::size_t last = line.size() - n;
    for (std::size_t i = 0; i <= last; ++i) {
        std::size_t j = 0;
        while (j < n && CharEqIgnoreCase(line[i + j], needle[j])){ 
            ++j;
        }
        if (j == n) {
            return true;
        }
    }
    return false;
}

}