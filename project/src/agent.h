#pragma once
#include <event_list.h>
#include <unordered_map>
#include "os_handle.h"

namespace nano_edr {
   
EventParts ConvertOsEvent(const os_event* ev);

class Agent {
 public:
    Agent(std::size_t window_size, bool quiet) : window_(window_size), quiet_(quiet), total_(0), comments_(0){};  // из --window-size и --quiet
    void HandleEvent(const Event& event);
    void PrintSummary() const;
    void CountComment();
    void CountMalformed();
    void CountRejected();
    static void Trampoline(const os_event* ev, void* ctx) noexcept;

 private:
    EventList window_;
    bool quiet_ = false;
    size_t total_ = 0;
    size_t comments_ = 0;
    size_t rejected_ = 0;
    size_t malformed_ = 0;
    const Event* prev_prev_event_ = nullptr;
    std::unordered_map<std::string, unsigned> types_;
    void PrintContext() const;
};
}  // namespace nano_edr