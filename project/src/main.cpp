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

#include <cstdio>
#include <print>
#include <string>

#include "file_source.h"
#include "os_source.h"
#include "agent.h"


bool ParseArgs(int argc, char** argv, bool& quiet, std::string& path, bool& file_mode, std::string& out) {
    if (argc < 2) {
        std::print(stderr,
    "использование: nano-edr [--quiet] [--out <файл>] <журнал.log>\n"
    "  --out <файл>  сохранить детекты также в файл\n");
        return false;
    }

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--quiet") {
            quiet = true;
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
        } else if (std::string(argv[i]) == "--out") {
            if (!out.empty()) {
                std::print(stderr, "--out задан дважды\n");
                return false;
            }
            if (i + 1 >= argc) {
                std::print(stderr, "ошибка: после --out нужен путь\n");
                return false;
            }
            out = argv[++i];
        } else if (std::string(argv[i]).starts_with("--")) {
            std::print(stderr, "неизвестная опция: {}\n", std::string(argv[i]));
            return false;
        }else {
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
        struct Options {
            bool quiet = false;
            std::string path;
            bool file_mode = false;
            std::string out;
        };

        Options options;
        if (!ParseArgs(argc, argv, options.quiet, options.path, options.file_mode, options.out)) {
            return 2;
        }

        nano_edr::Agent agent(options.quiet);
        std::unique_ptr<nano_edr::IEventSource> source;
        if (options.file_mode) {
            source = std::make_unique<nano_edr::FileSource>(options.path);
        } else {
            source = std::make_unique<nano_edr::OsSource>(options.path);
        }
        nano_edr::FanOutSink targets;
        targets.Add(nano_edr::ConsoleSink());
        if (!options.out.empty()) {
            targets.Add(nano_edr::FileSink(options.out));
        }
        agent.Subscribe(std::move(targets));

        source->Run(&agent);
        agent.PrintSummary();

        return 0;
    } catch (const std::exception& error) {
        std::print(stderr, "error: {}\n", error.what());
        return 1;
    }
}