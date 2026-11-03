#include "field_traits.h"
#include "text.h"

namespace nano_edr {

bool FieldTraits<std::string>::Parse(const std::string& text, std::string* out) {
    *out = text;
    return true;
}
bool FieldTraits<uint64_t>::Parse(const std::string& text, uint64_t* out) {
    uint64_t value = 0;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (ec != std::errc{} || ptr != text.data() + text.size()) {
        return false;   
    }
    *out = value;
    return true;
}
bool FieldTraits<bool>::Parse(const std::string& text, bool* out) {
    if (EqualsIgnoreCase(text, "true") || EqualsIgnoreCase(text, "1") || EqualsIgnoreCase(text, "yes")) {
        *out = true;
        return true;
    }
    if (EqualsIgnoreCase(text, "false") || EqualsIgnoreCase(text, "0") || EqualsIgnoreCase(text, "no")) {
        *out = false;
        return true;
    }
    return false;
}

}  // namespace nano_edr
