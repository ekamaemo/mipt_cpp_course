#include "os_source.h"
#include "event_source.h"
#include<print>


namespace nano_edr{
    
EventParts ConvertOsEvent(const os_event* ev) {
    EventParts parts;
    parts.ts = std::to_string(ev->ts);
    parts.type = ev->type != nullptr ? ev->type : "";
    parts.pid = (ev->pid != 0) ? std::to_string(ev->pid) : "";

    for (std::size_t i = 0; i < ev->field_count; ++i) {
        const os_kv& kv = ev->fields[i];
        parts.fields.push_back(Field{
            kv.key != nullptr ? kv.key : "",
            kv.value != nullptr ? kv.value : ""});
    }
    return parts;
}

void OsSource::Run(IEventHandler* handler) {
    handle_.Subscribe(&OsSource::Trampoline, handler);
    handle_.Start();

    os_status status;
    while ((status = handle_.Wait(100)) == OS_TIMEOUT) {
        handler->OnTick();
    }
    if (status != OS_OK) {
        throw std::runtime_error(std::string("os_wait: ") + os_status_str(status));
    }
}

void OsSource::Trampoline(const os_event* ev, void* ctx) noexcept {
    IEventHandler* handler = static_cast<IEventHandler*>(ctx);
    try {
        EventParts parts = ConvertOsEvent(ev);
        Event event(parts);
        handler->HandleEvent(event);
    } catch (const std::exception& error) {
        handler->OnBroken();
        std::print(stderr, "Error reason: {}\n", error.what());
    } catch (...) {
        handler->OnBroken();
        std::print(stderr, "Unknown error reason\n");
    }
}
}  // namespace nano_edr