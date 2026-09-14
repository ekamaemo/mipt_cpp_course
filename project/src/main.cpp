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
#include <map>
#include <vector>

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
    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "--quiet") {
            quiet= true;
        }
    }

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

        // Строки-комментарии в журнале начинаются с '#'. Они не события,
        // и детекта по ним быть не должно.
        if (!line.empty() && line[0] == '#') {
            ++comments;
            continue;
        }
        
        // >>> Здесь начинается занятие 1.1.
        //
        // Проверка признаков и печать детекта. Номер строки, который нужен
        // в выводе, — это lines.
        
        std::size_t i = 0;
        while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) {
            ++i;
        }

        if (i == line.size()) {
            continue;
        }

        ++total;

        const std::string key = "type=";
        std::size_t pos = line.find(key);
        if (pos != std::string::npos) {
            std::size_t start = pos + key.size();
            std::size_t end = start;
            while (end < line.size() && line[end] != ' ' && line[end] != '\t') {
                ++end;
            }
            const std::string type = line.substr(start, end - start);

            ++types[type];
        }

        for (const std::string& sig : signatures) {
            if (line.find(sig) != std::string::npos) {
                std::print("[DETECT] строка {}, признак {}: {}\n", lines, sig, line);
            }
        }

    }
    if (!quiet) {
        std::print("Всего событий: {}\n", total);
        for (const auto& [type, count] : types) {
            std::print(" {}: {}\n", type, count);
        }
    }
    return 0;
}
