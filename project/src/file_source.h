#pragma once
#include "event_source.h"


namespace nano_edr{
class FileSource : public IEventSource{
 public:
    explicit FileSource(const std::string& path): path_(path){}
    void Run(IEventHandler*) override;

 private:
    const std::string path_;
};
}