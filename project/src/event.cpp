#include <event.h>

namespace nano_edr {
Timestamp Event::Validate(const std::string& raw_ts, const std::string& type) {
    if (raw_ts.empty()) {
        throw std::invalid_argument("Пустое время");
    }
    if (type.empty()) {
        throw std::invalid_argument("Пустой тип события");
    }

    uint64_t parsedTs = 0;
    const char* begin = raw_ts.data();
    const char* end = raw_ts.data() + raw_ts.size();
    auto result = std::from_chars(begin, end, parsedTs);
    if (result.ec != std::errc{} || result.ptr != end) {
        throw std::invalid_argument("неверное время события: " + raw_ts);
    }
    return Timestamp{parsedTs};
}

Event::Event(EventParts&& parts)
    : ts_(Validate(parts.ts, parts.type)),
      raw_ts_(std::move(parts.ts)),
      type_(std::move(parts.type)),
      event_type_(EventTypeFromString(type_)),
      pid_(std::move(parts.pid)),
      fields_(std::move(parts.fields)) {}

Event::Event(const EventParts& parts)
    : ts_(Validate(parts.ts, parts.type)),
      raw_ts_(parts.ts),
      type_(parts.type),
      event_type_(EventTypeFromString(type_)),
      pid_(parts.pid),
      fields_(parts.fields) {}

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

EventType EventTypeFromString(const std::string& type) {
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