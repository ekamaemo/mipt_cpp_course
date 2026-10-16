#pragma once
#include "agent.h"


namespace nano_edr{
class FileSource {
 public:
    explicit FileSource(const std::string& path): path_(path){}
    void Run(Agent* agent);

 private:
    const std::string path_;
};
}