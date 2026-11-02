#pragma once
#include "event_source.h"
#include "os_handle.h"

namespace nano_edr {

EventParts ConvertOsEvent(const os_event* ev);
class OsSource : public IEventSource{
 public:
    explicit OsSource(const std::string& config_path)
        : handle_(config_path) {}

    void Run(IEventHandler*) override;
    static void Trampoline(const os_event* ev, void* ctx) noexcept;

 private:
    OsHandle handle_;
};
} 