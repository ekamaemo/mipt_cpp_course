#include "file_source.h"
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
}