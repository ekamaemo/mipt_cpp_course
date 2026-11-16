#include <string>

#include "detection.h"
#include "entity_model.h"
#include "event.h"
#include "os_handle.h"
#include "response.h"
#include "text.h"
#include "fields.h"

namespace nano_edr {
namespace {
bool IsSystemPath(std::string path){
    return StartsWithIgnoreCase(path, "C:\\Windows\\") || StartsWithIgnoreCase(path, "C:\\Program Files");
}
}

// --- ResponsePlanner -------------------------------------------------------

ResponsePlanner::ResponsePlanner(const EntityModel* model,
                                 std::string quarantine_dir)
    : model_(model), quarantine_dir_(quarantine_dir) {}

ResponsePlan ResponsePlanner::Plan(const Detection& detection) const {

    ResponsePlan plan;
    // процесс детекта и все его потомки; процесс детекта стоит первым
    const std::vector<std::string> tree =
        model_->GetProcessTree(detection.pid);
    for (const std::string& name : detection.actions) {
        if (name == "kill_process") {
            for (std::size_t i = tree.size(); i > 0; --i) {
                const std::string& pid = tree[i - 1];
                Action action;
                action.kind = ActionKind::kKillProcess;
                action.pid = pid;
                // запись есть у каждого процесса из tree;
                // start — ноль, если запуска процесса модель не видела
                action.expected_start = model_->FindProcess(pid)->start;
                action.reason = detection.rule;
                if (pid != detection.pid) {
                    action.reason += ", потомок " + detection.pid;
                }
                plan.Add(std::move(action));
            }
        } else if (name == "quarantine_source") {
            const FileRecord* source = model_->FindSpawnSource(detection.pid);
            if (source != nullptr && !IsSystemPath(source->path) && !QuarantinePath(source->path, detection.pid).empty()) {
                // сюда же — проверки из пункта 5
                Action action;
                action.kind = ActionKind::kQuarantineFile;
                action.path = source->path;
                action.to = QuarantinePath(source->path, detection.pid);
                action.reason = detection.rule + ", источник запуска";
                plan.Add(std::move(action));
            }
        } else if (name == "quarantine_image") {
            const ProcessRecord* source = model_->FindProcess(detection.pid);
            if (source != nullptr && !IsSystemPath(source->image) && !QuarantinePath(source->image, detection.pid).empty()) {
                Action action;
                action.kind = ActionKind::kQuarantineFile;
                action.path = source->image;
                action.to = QuarantinePath(source->image, detection.pid);
                action.reason = detection.rule + ", образ процесса";
                plan.Add(std::move(action));
            }
        } else if (name == "delete_dropped"){
            for (std::string pid: tree){
                const std::vector<std::string> source = model_->GetFilesCreatedBy(pid, true);
                for (std::string path : source){
                    if (!IsSystemPath(path)){
                        Action action;
                        action.kind = ActionKind::kDeleteFile;
                        action.path = path;
                        action.reason = detection.rule + ", принесён " + pid;
                        plan.Add(std::move(action));
                    }
                }
            }
        } else if (name == "delete_evidence") {
            for (const auto& event : detection.evidence){
                if (event == nullptr) {
                    continue;
                }
                std::string field_name = event->type() == "file_move" ? "to" : "path";
                const std::string* path = FindField(*event, field_name);
                if (path == nullptr || IsSystemPath(*path)) {
                    continue;
                }
                Action action;
                action.kind = ActionKind::kDeleteFile;
                action.path = *path;
                action.reason = detection.rule + ", файл из детекта";
                plan.Add(std::move(action));
            }
        }
        // незнакомое имя пропускается
    }
    return plan;
}

std::string ResponsePlanner::QuarantinePath(const std::string& path,
                                            const std::string& pid) const {
    if (quarantine_dir_.empty()){
        return "";
    }
    std::string file_name = path;
    if (path.rfind('\\') != std::string::npos){
        file_name = path.substr(path.rfind('\\') + 1, path.size());
    }
    return quarantine_dir_ + "\\" + pid + "-" + file_name;
}

// --- Responder -------------------------------------------------------------

Responder::Responder(OsHandle* os, bool dry_run) : os_(os), dry_run_(dry_run) {}

void Responder::Execute(const ResponsePlan&) {}

}  // namespace nano_edr
