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

#include "agent.h"
#include "copy_stats.h"
#include "edr_handle.h"
#include "field_traits.h"
#include "file_source.h"
#include "os_source.h"

struct Options {
    bool quiet = false;
    bool copies = false;
    bool dry_run = false;
    std::string path;
    bool file_mode = false;
    std::string out;
};

bool ParseArgs(int argc, char** argv, Options* options) {
    if (argc < 2) {
        std::print(stderr,
                   "использование: nano-edr [--quiet] [--out <файл>] <журнал.log>\n"
                   "             nano-edr [--quiet] [--out <файл>] --file <журнал.log>\n"
                   "  --file <журнал.log>  читать события из файла\n"
                   "  --out <файл>         сохранить детекты также в файл\n");
        return false;
    }
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--quiet") {
            options->quiet = true;
        } else if (std::string(argv[i]) == "--copies") {
            options->copies = true;
        } else if (std::string(argv[i]) == "--dry-run"){
            options->dry_run = true;
        } else if (std::string(argv[i]) == "--file") {
            if (options->file_mode) {
                std::print(stderr, "--file задан дважды\n");
                return false;
            }
            if (!options->path.empty()) {
                std::print(stderr, "путь задан дважды\n");
                return false;
            }
            if (i + 1 >= argc) {
                std::print(stderr, "ошибка: после --file нужен путь\n");
                return false;
            }
            options->file_mode = true;
            options->path = argv[++i];
        } else if (std::string(argv[i]) == "--out") {
            if (!options->out.empty()) {
                std::print(stderr, "--out задан дважды\n");
                return false;
            }
            if (i + 1 >= argc) {
                std::print(stderr, "ошибка: после --out нужен путь\n");
                return false;
            }
            options->out = argv[++i];
        } else if (std::string(argv[i]).starts_with("--")) {
            std::print(stderr, "неизвестная опция: {}\n", std::string(argv[i]));
            return false;
        } else {
            if (!options->path.empty()) {
                std::print(stderr, "путь задан дважды\n");
                return false;
            }
            options->path = std::string(argv[i]);
        }
    }

    if (options->path.empty()) {
        std::print(stderr, "не указан путь\n");
        return false;
    }

    return true;
}

int main(int argc, char** argv) {
    try {
        Options options;
        if (!ParseArgs(argc, argv, &options)) {
            return 2;
        }
        nano_edr::EdrHandle edr(options.path);
        const std::string text = edr.Config("dedup_period_ms", "10000");
        uint64_t dedup_period_ms = 0;
        if (!nano_edr::FieldTraits<uint64_t>::Parse(text, &dedup_period_ms)) {
            std::print(stderr, "[WARN] dedup_period_ms не разбирается: {}\n", text);
            dedup_period_ms = 10000;
        }
        std::unique_ptr<nano_edr::IEventSource> source;
        if (options.file_mode) {
            source = std::make_unique<nano_edr::FileSource>(options.path);
        } else {
            source = std::make_unique<nano_edr::OsSource>(options.path);
        }
        nano_edr::Agent agent(options.quiet, dedup_period_ms);
        nano_edr::FanOutSink targets;
        targets.Add(nano_edr::EdrSink(&edr));
        targets.Add(nano_edr::ConsoleSink());
        if (!options.out.empty()) {
            targets.Add(nano_edr::FileSink(options.out));
        }
        agent.Subscribe(std::move(targets));
        agent.EnableResponse(source->os(), options.dry_run,
                     edr.Config("quarantine_dir", "C:\\quarantine"));

        source->Run(&agent);
        if (edr.Flush() != EDR_OK) {
            std::print(stderr, "[WARN] детекты не отправлены\n");
        }
        agent.PrintSummary();
        if (options.copies) {
            std::print("[COPY] строк события скопировано {}, перемещено {}\n"
                       "[COPY] записей WindowEntry скопировано {}, перемещено {}\n"
                       "[COPY] событий скопировано в детекты {}\n"
                       "[COPY] копий всего {}\n",
                       nano_edr::copy_stats().event_strings_copied, nano_edr::copy_stats().event_strings_moved,
                       nano_edr::copy_stats().window_copied, nano_edr::copy_stats().window_moved,
                       nano_edr::copy_stats().evidence_copied,
                       nano_edr::copy_stats().total());
        }
        return 0;
    } catch (const std::exception& error) {
        std::print(stderr, "error: {}\n", error.what());
        return 1;
    }
}