#include "sources.h"

#include <fstream>

#include "parse.h"

namespace nano_edr {

void FileSource::Run(Agent* agent) {
    std::ifstream in(path_);
    if (!in) {
        throw std::runtime_error("не удалось открыть журнал: " + path_);
    }
    std::string line;
    while (std::getline(in, line)) {
        if (IsBlankOrComment(line)) {
            agent->CountComment();
            continue;
        }

        EventParts parts;
        if (!ParseEventParts(line, &parts)) {
            agent->CountMalformed();
            continue;
        }

        try {
            Event event(parts);
            agent->HandleEvent(event);
        } catch (const std::exception&) {
            agent->CountRejected();
            continue;
        }
    }
}

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