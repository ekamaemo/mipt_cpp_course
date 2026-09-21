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
#include "parse.h"
#include "event_list.h"
#include <charconv>
#include<map>
#include<vector>

int main(int argc, char** argv) {
    // Аргументы разбираются грубо: путь к журналу и ничего больше. Остальное,
    // включая --quiet, добавляется по заданию.

    if (argc < 2) {
        std::print(stderr, "использование: nano-edr <журнал.log>\n");
        return 2;
    }

    std::ifstream log(argv[1]);
    if (!log) {
        std::print(stderr, "не удалось открыть журнал: {}\n", argv[1]);
        return 2;
    }

    bool quiet = false;
    std::size_t window_size = 64;
    for (int i = 0; i < argc; ++i) {
        if (std::string(argv[i]) == "--quiet") {
            quiet= true;
        } else if (std::string(argv[i])=="--window-size"){
            if (i + 1 >= argc) {
                std::print(stderr, "ошибка: после --window-size нужно указать число\n");
                return 2;
            }
            std::string value = argv[++i];
            std::size_t parsed = 0;
            auto result = std::from_chars(value.data(), value.data() + value.size(), parsed);
            if (result.ec != std::errc{} || result.ptr != value.data() + value.size()){
                std::print(stderr, "ошибка: --window-size требует целое число");
                return 2;
            }
            window_size = parsed;
        }
    }

    nano_edr::EventList window_events;
    window_events.capacity = window_size;

    long long lines = 0;
    long long comments = 0;
    long long total = 0;
    std::string line;
    std::map<std::string, unsigned> types;

    const std::vector<std::string> signatures = {
        "wscript.exe", ".locked", "certutil.exe", "\\Startup\\",
    };

    while (std::getline(log, line)) {
        // Счётчик увеличивается до всех проверок: он считает строки файла,
        // а не события. Номер, посчитанный по событиям, бесполезен — по нему
        // нельзя открыть файл и посмотреть.
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
        ++types[event.type];

        
        for (const std::string& sig : signatures) {
            bool detected = false;
            for (const nano_edr:: Field& field : event.fields) {
                if (field.value.find(sig) != std::string::npos){
                    detected = true;
                    break;
                }
            }

            if (detected) {
                if (!quiet) {
                    nano_edr::Event* prev_prev_event = nullptr;
                    nano_edr::Event* prev_event = nullptr;
                    
                    nano_edr::EventNode* current = window_events.head;

                    while (current != nullptr){
                        prev_prev_event = prev_event;
                        prev_event = &current->event;
                        current = current->next;
                    }

                    if (prev_prev_event != nullptr){
                        std::print(
                            "[CTX] {}: ts={} type={} pid={}\n",
                            -2,
                            prev_prev_event->ts,
                            prev_prev_event->type,
                            prev_prev_event->pid
                        );
                    }

                    if (prev_event != nullptr){
                        std::print(
                            "[CTX] {}: ts={} type={} pid={}\n",
                            -1,
                            prev_event->ts,
                            prev_event->type,
                            prev_event->pid
                        );
                    }
                }
                std::print("[DETECT] строка {}, признак {}: {}\n", lines, sig, line);
            }
        }
        nano_edr::ListPushBack(&window_events, &event);

    }
    if (!quiet) {
        std::print("Всего событий: {}, комментариев: {}\n", total, comments);
        for (const auto& [type, count] : types) {
            std::print(" {}: {}\n", type, count);
        }
    }
    return 0;
}
