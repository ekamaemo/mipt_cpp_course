#include<event_list.h>

namespace nano_edr{

class Agent {
 public:
    Agent(std::size_t window_size, bool quiet);  // из --window-size и --quiet
    void HandleEvent(const Event& event);
    void PrintSummary() const;  // сводка в конце прогона
    // ...
 private:
    EventList window_;  // последние события, не больше window_size
    // счётчики для сводки
};

class FileSource {
 public:
    explicit FileSource(const std::string& path);
    void Run(Agent* agent);  // строка → EventParts → Event → HandleEvent
    // ...
};
}