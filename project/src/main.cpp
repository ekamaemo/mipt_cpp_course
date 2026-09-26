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
#include <print>
#include <string>
#include <charconv>
#include<map>
#include<vector>

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


void ProcessLog(const std::string& path, std::size_t window_size,
                long long& lines,
                long long& comments, 
                long long& total, std::map<std::string, unsigned>& types){
    std::ifstream log(path);

    nano_edr::EventList window_events;
    window_events.capacity = window_size;

    const nano_edr::Rule* rules = nano_edr::AgentRules();
    const std::size_t rule_count = nano_edr::AgentRuleCount();

    std::string line;
    while (std::getline(log, line)) {
        ++lines;

        if (nano_edr::IsBlankOrComment(&line)){
            comments++;
            continue;
        }

        nano_edr::Event event;
        if (!nano_edr::ParseEventLine(&line, &event)){
            continue;
        }

        total++;
        types[event.type]++;
        nano_edr::CheckRules(event, rules, rule_count);

        nano_edr::ListPushBack(&window_events, &event);

    }
}


int main(int argc, char** argv) {
    // Аргументы разбираются грубо: путь к журналу и ничего больше. Остальное,
    // включая --quiet, добавляется по заданию.

    bool quiet = false;
    std::size_t window_size = 64;
    std::string path;
    if (!ParseArgs(argc, argv, quiet, window_size, path)){
        return 2;
    }
    
    // проверка, что лог открывается
    std::ifstream log(argv[1]);
    if (!log) {
        std::print(stderr, "не удалось открыть журнал: {}\n", argv[1]);
        return 2;
    }

    long long lines = 0;
    long long comments = 0;
    long long total = 0;

    std::map<std::string, unsigned> types;
    try {
        ProcessLog(path, window_size, lines, comments, total, types);
    } catch (const std::exception& error){
        std::print(stderr, "error: {}\n", error.what());
        return 1;
    }

    if (!quiet) {
        std::print("Всего событий: {}, комментариев: {}\n", total, comments);
        for (const auto& [type, count] : types) {
            std::print(" {}: {}\n", type, count);
        }
    }
    return 0;
}
