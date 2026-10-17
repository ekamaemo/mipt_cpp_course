#include "os_source.h"

namespace nano_edr{
    void OsSource::Run(Agent* agent) {
    handle_.Subscribe(&Agent::Trampoline, agent);
    handle_.Start();

    os_status status;
    while ((status = handle_.Wait(100)) == OS_TIMEOUT) {
        // тик
    }
    if (status != OS_OK) {
        throw std::runtime_error(std::string("os_wait: ") + os_status_str(status));
    }
}
}  // namespace nano_edr