#include "fields.h"

#include <cctype>
#include <cstdint>
#include <stdexcept>

#include "text.h"

namespace nano_edr {
namespace {
std::string LowerString(const std::string& str) {
    std::string res;
    res.reserve(str.size());
    for (char c : str) {
        res.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return res;
}
}

// возвращает nullptr, если поля нет. Это не ошибка, потому что есть
// необязательные поля. Вызывающий должен проверять на nullptr.
const std::string* FindField(const Event& event, const std::string& key) {
    if (key == "ts") {
        return &event.raw_ts();
    }
    if (key == "type") {
        return &event.type();
    }
    if (key == "pid") {
        return &event.pid();
    }
    for (const Field& field : event.fields()) {
        if (field.key == key) {
            return &(field.value);
        }
    }
    return nullptr;
}

const std::string& GetRequiredField(const Event& event, const std::string& key) {
    const std::string* val = FindField(event, key);
    if (val == nullptr) {
        throw std::invalid_argument("required field missing: " + key);
    }
    return *val;
}

bool GetIntField(const Event& event, const std::string& key, uint64_t* out) {
    const std::string* value = FindField(event, key);
    if (value == nullptr) {
        return false;
    }

    uint64_t parsed = 0;
    const char* begin = value->data();
    const char* end = value->data() + value->size();
    auto result = std::from_chars(begin, end, parsed);
    if (result.ec != std::errc{} || result.ptr != end) {
        return false;
    }

    *out = parsed;
    return true;
}

uint64_t GetIntField(const Event& event, const std::string& key, uint64_t fallback) {
    uint64_t value = 0;
    if (GetIntField(event, key, &value)) {
        return value;
    }
    return fallback;
}

bool IsProcessStart(const Event& event) {
    return event.type() == "process_start";
}

bool IsFileWrite(const Event& event) {
    return event.type() == "file_write";
}

bool IsNetConnect(const Event& event) {
    return event.type() == "net_connect";
}


bool PathEndsWith(const Event& event, const std::string& suffix) {
    const std::string* path = FindField(event, "path");
    if (path == nullptr) {
        return false;
    }
    return EndsWithIgnoreCase(*path, suffix);
}

bool CommandLineContains(const Event& event, const std::string& needle) {
    const std::string* cmdline = FindField(event, "cmdline");
    if (cmdline == nullptr) {
        return false;
    }
    return ContainsIgnoreCase(*cmdline, needle);
}

std::string NormalizePath(const std::string& path) {
    std::string path_new;
    path_new.reserve(path.size());
    for (char c : LowerString(path)) {
        if (c == '/' || c == '\\') {
            if (path_new.empty() || path_new.back() != '\\') {
                path_new.push_back('\\');
            }
        } else {
            path_new.push_back(c);
        }
    }
    return path_new;
}

}  // namespace nano_edr