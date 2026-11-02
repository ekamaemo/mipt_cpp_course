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
#include <agent_rules.h>
#include <os_handle.h>
#include <rules.h>

#include <charconv>
#include <cstdio>
#include <print>
#include <string>

#include "file_source.h"
#include "os_source.h"
#include "agent.h"


bool ParseArgs(int argc, char** argv, bool& quiet, std::size_t& window_size, std::string& path, bool& file_mode) {
    if (argc < 2) {
        std::print(stderr, "использование: nano-edr <журнал.log>\n");
        return false;
    }

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--quiet") {
            quiet = true;
        } else if (std::string(argv[i]) == "--window-size") {
            if (i + 1 >= argc) {
                std::print(stderr, "ошибка: после --window-size нужно указать число\n");
                return false;
            }
            std::string value = argv[++i];
            std::size_t parsed = 0;
            auto result = std::from_chars(value.data(), value.data() + value.size(), parsed);
            if (result.ec != std::errc{} || result.ptr != value.data() + value.size()) {
                std::print(stderr, "ошибка: --window-size требует целое число\n");
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
        } else if (std::string(argv[i]).starts_with("--")) {
            std::print(stderr, "неизвестная опция: {}\n", std::string(argv[i]));
            return false;
        } else {
            if (!path.empty()) {
                std::print(stderr, "путь задан дважды\n");
                return false;
            }
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
        std::unique_ptr<nano_edr::IEventSource> source;
        if (file_mode) {
            source = std::make_unique<nano_edr::FileSource>(path);
        } else {
            source = std::make_unique<nano_edr::OsSource>(path);
        }
        source->Run(&agent);
        agent.PrintSummary();

        return 0;
    } catch (const std::exception& error) {
        std::print(stderr, "error: {}\n", error.what());
        return 1;
    }
}