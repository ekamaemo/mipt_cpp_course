// Каркас агента: читает журнал событий построчно и считает строки.
//
// Это заготовка занятия 1.1, а не решение. Детектов она не ищет — их вы
// добавите здесь же, в отмеченном месте ниже. Формат строки детекта, список
// признаков и правило про их порядок заданы в постановке занятия: по ним
// сравниваются эталоны.
//
// Весь код лежит в main, и на этом занятии так и надо: функции появятся
// на занятии 1.2, ссылки — на 1.3. Разбор аргументов, коды возврата и флаг
// --quiet — часть задания.
//
// Запуск:
//   nano-edr <журнал.log>
#include <cstdio>
#include <fstream>
#include <optional>
#include <print>
#include <string>
#include <charconv>
#include<unordered_map>

#include "parse.h"
#include "event_list.h"
#include<rules.h>
#include<agent_rules.h>
#include<os_handle.h>


namespace nano_edr {
    class Agent{
        public :
            Agent(std::size_t window_size, bool quiet): window_(window_size), window_size_(window_size), quiet_(quiet), total_(0), comments_(0) {};
            void HandleEvent(const Event& event){
                const nano_edr::Rule* rules = nano_edr::AgentRules();
                const std::size_t rule_count = nano_edr::AgentRuleCount();

                ++total_;
                ++types_[event.type()];
                size_t n = nano_edr::CheckRules(event, rules, rule_count);
                if (n != 0 && !quiet_){
                    if (prev_prev_event_.has_value() && window_size_ >= 2){
                        std::print(
                            "[CTX] {}: ts={} type={} pid={}\n",
                            -2,
                            prev_prev_event_->raw_ts(),
                            prev_prev_event_->type(),
                            prev_prev_event_->pid()
                            );
                    }
                    
                    if (window_.back() != nullptr){
                        std::print(
                            "[CTX] {}: ts={} type={} pid={}\n",
                            -1,
                            window_.back()->event.raw_ts(),
                            window_.back()->event.type(),
                            window_.back()->event.pid()
                            );
                    }
                }
                if (window_.back() != nullptr) {
                    prev_prev_event_ = window_.back()->event;  
                }

                window_.PushBack(event);
            }

            void PrintSummary() const{
                if (quiet_){
                    return;
                }
                std::print("Всего событий: {}, всего комментариев: {}\n", total_, comments_);
                for (const auto& [type, count] : types_) {
                    std::print(" {}: {}\n", type, count);
                }
            } 

            void CountComment() { 
                ++comments_; 
            }

            static void Trampoline(const os_event* ev, void* ctx) noexcept {
                try {
                    Agent* agent = static_cast<Agent*>(ctx);
                    agent->HandleOsEvent(ev);
                } catch (...) {
                    // что ловить то надо
                }
            }

            void HandleOsEvent(const os_event* ev){
                EventParts parts;
                parts.ts = std::to_string(ev->ts);
                parts.type = ev->type != nullptr ? ev->type : "";
                parts.pid = (ev->pid != 0) ? std::to_string(ev->pid) : "";

                for (std::size_t i = 0; i < ev->field_count; ++i) {
                        const os_kv& kv = ev->fields[i];
                        parts.fields.push_back(Field{
                            kv.key   != nullptr ? kv.key   : "",
                            kv.value != nullptr ? kv.value : ""
                        });
                    }

                Event event(parts);
                HandleEvent(event);
            }

        private:
            EventList window_;
            std::size_t window_size_;
            bool quiet_;
            long long total_;
            long long comments_;
            std::optional<Event> prev_prev_event_;
            std::unordered_map<std::string, unsigned> types_;
    };

    class FileSource {
        public: 
            explicit FileSource(const std::string& path) : path_(path) {};
            void Run(Agent* agent){
                std::ifstream in(path_);
                if (!in){
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
                        continue; 
                    }

                    try {
                        Event event(parts);
                        agent->HandleEvent(event);
                    } catch (const std::exception&) {
                        continue; 
                    }
                }
            }
        private:
            const std::string path_;
    };


    class OsSource {
        public:
            explicit OsSource(const std::string& config_path)
                : handle_(config_path) {}

            void Run(Agent* agent) {
                handle_.Subscribe(&Agent::Trampoline, agent);
                handle_.Start();

                os_status status;
                while ((status = handle_.Wait(100)) == OS_TIMEOUT) {
                    // тик
                }
                if (status != OS_OK) {
                    throw std::runtime_error(std::string("os_wait: ")
                                            + os_status_str(status));
                }
            }

        private:
            OsHandle handle_;
        };
}



bool ParseArgs(int argc, char** argv, bool& quiet, std::size_t& window_size, std::string& path, bool& file_mode){

    if (argc < 2) {
        std::print(stderr, "использование: nano-edr <журнал.log>\n");
        return false;
    }

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--quiet") {
            quiet= true;
        } else if (std::string(argv[i])=="--window-size"){
            if (i + 1 >= argc) {
                std::print(stderr, "ошибка: после --window-size нужно указать число\n");
                return false;
            }
            std::string value = argv[++i];
            std::size_t parsed = 0;
            auto result = std::from_chars(value.data(), value.data() + value.size(), parsed);
            if (result.ec != std::errc{} || result.ptr != value.data() + value.size()){
                std::print(stderr, "ошибка: --window-size требует целое число");
                return false;
            }
            window_size = parsed;
        } else if (std::string(argv[i]) == "--file") {
            if (file_mode) {
                std::print(stderr, "--file задан дважды\n");
                return false;
            }
            if (!path.empty()) {                         
                std::print(stderr, "путь задан дважды\n");
                return false;
            }
            if (i + 1 >= argc) {
                std::print(stderr, "ошибка: после --file нужен путь\n");
                return false;
            }
            file_mode = true;
            path = argv[++i];
        }  else if (std::string(argv[i]).starts_with("--")) {
            std::print(stderr, "неизвестная опция: {}\n", std::string(argv[i]));
            return false;
        } else {
            if (!path.empty()) { std::print(stderr, "путь задан дважды\n"); return false; }
            path = std::string(argv[i]);
        }
    }

    if (path.empty()) {
        std::print(stderr, "не указан путь\n");
        return false;
    }

    return true;
}



int main(int argc, char** argv) {
    try {
        bool quiet = false;
        std::size_t window_size = 64;
        std::string path;
        bool file_mode = false;

        if (!ParseArgs(argc, argv, quiet, window_size, path, file_mode)) {
            return 2;
        }

        nano_edr::Agent agent(window_size, quiet);
        if (file_mode) {
            nano_edr::FileSource source(path);
            source.Run(&agent);
        } else {
            nano_edr::OsSource source(path);
            source.Run(&agent);
        }
        agent.PrintSummary();

        return 0;
    } catch (const std::exception& error) {
        std::print(stderr, "error: {}\n", error.what());
        return 1;
    }
}