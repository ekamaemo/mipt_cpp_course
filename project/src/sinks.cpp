#include <stdexcept>
#include <string>

#include "alert_sink.h"
#include "dedup_sink.h"
#include "detection.h"

#include<print>


namespace nano_edr {

// --- ConsoleSink -----------------------------------------------------------

void ConsoleSink::operator()(const Detection& detection) const {
    std::print("{}\n", ToString(detection));
}

// --- FileSink --------------------------------------------------------------

FileSink::FileSink(const std::string& path) : out_(path) {
    if (!out_.is_open()) {
        throw std::runtime_error("не удалось открыть файл: " + path);
    }
}

void FileSink::operator()(const Detection& detection) {
    out_ << ToString(detection) << '\n';
}

// --- CountingSink ----------------------------------------------------------

void CountingSink::operator()(const Detection& detection) const {
    ++counter_->total;
    counter_->seen.push_back(detection);
}

// --- FanOutSink ------------------------------------------------------------

void FanOutSink::Add(AlertSink sink) {
    sinks_.push_back(std::move(sink));
}

void FanOutSink::operator()(const Detection& detection) const {
    for (const AlertSink& sink : sinks_) {
        sink(detection);
    }
}

// --- DedupSink -------------------------------------------------------------

DedupSink::DedupSink(AlertSink inner, uint64_t period_ms, std::size_t* suppressed)
    : inner_(std::move(inner)), period_ms_(period_ms), suppressed_(suppressed) {}

void DedupSink::operator()(const Detection& detection) {
    if (period_ms_ == 0) {
        inner_(detection);
        return;
    }
    for (Detect& d : last_forwarded_){
        if (d.rule == detection.rule && d.pid == detection.pid){
            if(detection.ts.ms - d.ts_ms_first <= period_ms_){
                if (suppressed_ != nullptr) {
                    ++(*suppressed_);
                }
                return;
            } else {
                inner_(detection);
                d.ts_ms_first = detection.ts.ms;
                return; // чтобы не заполнять память лишним - меняем последнее время
            }
        }
    }
    // сюда попадаем если нужно создать новую запись о праивле с процессом
    inner_(detection);
    last_forwarded_.push_back({detection.rule, detection.pid, detection.ts.ms});
}

}  // namespace nano_edr
