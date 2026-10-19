#include "file_source.h"
#include <fstream>
#include "parse.h"

namespace nano_edr {

void FileSource::Run(IEventHandler* handler) {
    std::ifstream in(path_);
    if (!in) {
        throw std::runtime_error("не удалось открыть журнал: " + path_);
    }
    std::string line;
    while (std::getline(in, line)) {
        if (IsBlankOrComment(line)) {
            handler->OnSkipped();
            continue;
        }

        EventParts parts;
        if (!ParseEventParts(line, &parts)) {
            handler->OnBroken();
            continue;
        }

        try {
            Event event(parts);
            handler->HandleEvent(event);
        } catch (const std::exception&) {
            handler->OnBroken();
            continue;
        }
    }
}
}