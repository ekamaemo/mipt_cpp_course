#include<event.h>

namespace nano_edr {
    Event::Event(const EventParts& parts) {
        if (parts.ts.empty()){
            throw std::invalid_argument("Пустое время");
        }
        if (parts.type.empty()){
            throw std::invalid_argument("Пустой тип события");
        }

        uint64_t parsedTs;
        const char* begin = parts.ts.data();
        const char* end   = parts.ts.data() + parts.ts.size();
        auto result = std::from_chars(begin, end, parsedTs);
        if (result.ec != std::errc{} || result.ptr != end) {
            throw std::invalid_argument("неверное время события: " + parts.ts);
        }
        ts_ = Timestamp{parsedTs};
        raw_ts_ = parts.ts;
        type_ = parts.type;
        pid_ = parts.pid;
        fields_ = parts.fields;
    };

    std::string ToString(const Event& event){
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
}