#include <fields.h>

#include <cctype>
#include <cstdint>
#include <stdexcept>

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

void ReplacePart(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos;
    start_pos = str.find(from);
    if (start_pos != std::string::npos) {
        str.replace(start_pos, from.size(), to);
    }
}
}  // namespace

// возвращает nullptr, если поля нет. Это не ошибка, потому что есть необязательные поля. Вызывающий функции должен проверять на nullptr.
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

// если нет поля, которое должно быть, функция бросает ошибку std::invalid_argument, так как отсутствие поля - нарушение формата
const std::string& GetRequiredField(const Event& event, const std::string& key) {
    const std::string* val = FindField(event, key);
    if (val == nullptr) {
        throw std::invalid_argument("required field missing: " + key);
    }
    return *val;
}

// возвращает false, если строка пустая, или она содержит не цифры, или если происходит переполнение uint64_t.
// если строка - полностью число, влезающее в uint64_t, то оно пишется в *out с true
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

// если поле некорректное, то об ошибке не сообщает, а возвращает fallback (значение по умолчанию)
// это нужно для полей, у которых нормально отсутствие полей - типа ppid
uint64_t GetIntField(const Event& event, const std::string& key, uint64_t fallback) {
    uint64_t value = 0;
    if (GetIntField(event, key, &value)) {
        return value;
    }
    return fallback;
}

// предикаты

bool IsProcessStart(const Event& event) {
    return event.type() == "process_start";
}

bool IsFileWrite(const Event& event) {
    return event.type() == "file_write";
}

bool IsNetConnect(const Event& event) {
    return event.type() == "net_connect";
}

// не сообщает об обшибках, любая строка приводится к одному виду в копию, не изменяя оригинал
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

    ReplacePart(path_new, "%temp%", "\\appdata\\local\\temp");
    ReplacePart(path_new, "%tmp%", "\\appdata\\local\\temp");
    return path_new;
}

bool PathEndsWith(const Event& event, const std::string& suffix) {
    const std::string* path = FindField(event, "path");
    if (path == nullptr) {
        return false;
    }

    std::string normal_path = NormalizePath(*path);
    std::string normal_suffix = NormalizePath(suffix);
    return normal_path.ends_with(normal_suffix);
}

bool CommandLineContains(const Event& event, const std::string& needle) {
    const std::string* cmdline = FindField(event, "cmdline");
    if (cmdline == nullptr) {
        return false;
    }
    std::string normal_cmdline = NormalizePath(*cmdline);
    return normal_cmdline.find(NormalizePath(needle)) != std::string::npos;
}

}  // namespace nano_edr
