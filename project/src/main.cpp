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
#include <mutex>
#include <print>
#include <string>
#include <charconv>
#include<unordered_map>

#include "parse.h"
#include "event_list.h"
#include<rules.h>
#include<agent_rules.h>


bool ParseArgs(int argc, char** argv, bool& quiet, std::size_t& window_size, std::string& path){

    if (argc < 2) {
        std::print(stderr, "использование: nano-edr <журнал.log>\n");
        return false;
    }
    path = argv[1];
        // проверка, что лог открывается
    std::ifstream log(path);
    if (!log) {
        std::print(stderr, "не удалось открыть журнал: {}\n", argv[1]);
        return false;
    }

    for (int i = 0; i < argc; ++i) {
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
        }
    }

    return true;
}

struct Guard 
{
    Guard()
{
 node = new nano_edr::EventNode();
}
    ~Guard(){
        delete node;
    }

    nano_edr::EventNode* node = nullptr;
};


void ProcessLog(const std::string& path, std::size_t window_size, bool quiet,
                long long& lines,
                long long& comments, 
                long long& total, std::unordered_map<std::string, unsigned>& types){
    std::ifstream log(path);

    nano_edr::EventList window_events;
    window_events.capacity = window_size;

    {
        auto node = new nano_edr::EventNode();
        Guard guard;
    }

    const nano_edr::Rule* rules = nano_edr::AgentRules();
    const std::size_t rule_count = nano_edr::AgentRuleCount();

    std::string line;
    nano_edr::Event* prev_prev_event = nullptr;
    
    while (std::getline(log, line)) {
        ++lines;

        if (nano_edr::IsBlankOrComment(&line)){
            ++comments;
            continue;
        }

        nano_edr::Event event;
        if (!nano_edr::ParseEventLine(&line, &event)){
            continue;
        }

        ++total;
        ++types[event.type];
        size_t n = nano_edr::CheckRules(event, rules, rule_count);
        if (n != 0 && !quiet){
            if (prev_prev_event != nullptr && window_size >= 2){
                std::print(
                    "[CTX] {}: ts={} type={} pid={}\n",
                    -2,
                    prev_prev_event->ts,
                    prev_prev_event->type,
                    prev_prev_event->pid
                    );
            }
            
            if (window_events.tail != nullptr){
                std::print(
                    "[CTX] {}: ts={} type={} pid={}\n",
                    -1,
                    window_events.tail->event.ts,
                    window_events.tail->event.type,
                    window_events.tail->event.pid
                    );
            }
        }
        prev_prev_event = &(window_events.tail->event);


        nano_edr::ListPushBack(&window_events, &event);

    }
}


int main(int argc, char** argv) {
    try {
        bool quiet = false;
        std::size_t window_size = 64;
        std::string path;
        if (!ParseArgs(argc, argv, quiet, window_size, path)){
            return 2;
        }
        
        long long lines = 0;
        long long comments = 0;
        long long total = 0;

        std::unordered_map<std::string, unsigned> types;
        
        ProcessLog(path, window_size, quiet, lines, comments, total, types);
        
        if (!quiet) {
            std::print("Всего событий: {}, комментариев: {}\n", total, comments);
            for (const auto& [type, count] : types) {
                std::print(" {}: {}\n", type, count);
            }
        }
        return 0;

    } catch (const std::exception& error) {
        std::print(stderr, "error: {}\n", error.what());
        return 1;
    }
}
