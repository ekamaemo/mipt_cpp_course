#include <event.h>

namespace nano_edr {
Event::Event(const EventParts& parts) {
    if (parts.ts.empty()) {
        throw std::invalid_argument("Пустое время");
    }
    if (parts.type.empty()) {
        throw std::invalid_argument("Пустой тип события");
    }

    uint64_t parsedTs = 0;
    const char* begin = parts.ts.data();
    const char* end = parts.ts.data() + parts.ts.size();
    auto result = std::from_chars(begin, end, parsedTs);
    if (result.ec != std::errc{} || result.ptr != end) {
        throw std::invalid_argument("неверное время события: " + parts.ts);
    }
    ts_ = Timestamp{parsedTs};
    raw_ts_ = parts.ts;
    type_ = parts.type;
    pid_ = parts.pid;
    fields_ = parts.fields;
    event_type_ = EventTypeFromString(type_);
}

std::string ToString(const Event& event) {
    std::string result;
    result += "ts=" + event.raw_ts();
    result += " type=" + event.type();
    if (!event.pid().empty()) {
        result += " pid=" + event.pid();
    }
    for (const Field& f : event.fields()) {
        result += " " + f.key + "=" + f.value;
    }
    return result;
}
std::ostream& operator<<(std::ostream& out, const Event& event) {
    return out << ToString(event);
}

EventType EventTypeFromString(const std::string &type){
    if (type == "process_start") {
        return EventType::kProcessStart;
    }
    if (type == "process_end") {
        return EventType::kProcessEnd;
    }
    if (type == "file_create") {
        return EventType::kFileCreate;
    }
    if (type == "file_write") {
        return EventType::kFileWrite;
    }
    if (type == "file_delete") {
        return EventType::kFileDelete;
    }
    if (type == "file_move") {
        return EventType::kFileMove;
    }
    if (type == "net_connect") {
        return EventType::kNetConnect;
    }
    return EventType::kOther;
}
}  // namespace nano_edr