#include "agent.h"

namespace nano_edr {
class FileSource {
 public:
    explicit FileSource(const std::string& path): path_(path){}
    void Run(Agent* agent);

 private:
    const std::string path_;
};

class OsSource {
 public:
    explicit OsSource(const std::string& config_path)
        : handle_(config_path) {}

    void Run(Agent* agent);

 private:
    OsHandle handle_;
};
}  // namespace nano_edr