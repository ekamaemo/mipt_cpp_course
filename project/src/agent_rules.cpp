#include "agent_rules.h"
#include "rule_engine.h"
#include"make_rule.h"
#include"combinators.h"

#include <memory>
#include <utility>
#include <vector>

namespace nano_edr {
namespace {

void AddScriptHostFromTempRule(RuleEngine& engine) {
    auto rule = MakeRule<MatchRule>("script_host_from_temp", Severity::kHigh);
    rule->AddCondition(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kProcessStart}));

    auto image = std::make_unique<AnyOf>();
    image->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
    image->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
    rule->AddCondition(std::move(image));

    auto cmdline = std::make_unique<AnyOf>();
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "\\appdata\\local\\temp\\"));
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "/appdata/local/temp/"));
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "\\windows\\temp\\"));
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "/windows/temp/"));
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "%temp%"));
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "%tmp%"));
    rule->AddCondition(std::move(cmdline));

    rule->AddAction("kill_process");
    engine.AddRule(std::move(rule));
}

void AddLolbinDownloadRule(RuleEngine& engine) {
    auto rule = MakeRule<MatchRule>("lolbin_download", Severity::kHigh);
    rule->AddCondition(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kProcessStart}));

    auto image = std::make_unique<AnyOf>();
    image->Add(std::make_unique<FieldEndsWith>("image", "certutil.exe"));
    image->Add(std::make_unique<FieldEndsWith>("image", "bitsadmin.exe"));
    rule->AddCondition(std::move(image));

    auto cmdline = std::make_unique<AnyOf>();
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "urlcache"));
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "transfer"));
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "http:"));
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "https:"));
    rule->AddCondition(std::move(cmdline));

    rule->AddAction("kill_process");
    engine.AddRule(std::move(rule));
}

void AddHiddenPowershellRule(RuleEngine& engine) {
    auto rule = MakeRule<MatchRule>("hidden_powershell", Severity::kMedium);
    rule->AddCondition(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kProcessStart}));

    auto image = std::make_unique<AnyOf>();
    image->Add(std::make_unique<FieldEndsWith>("image", "powershell.exe"));
    image->Add(std::make_unique<FieldEndsWith>("image", "pwsh.exe"));
    rule->AddCondition(std::move(image));

    auto cmdline = std::make_unique<AnyOf>();
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "-w hidden"));
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "-windowstyle hidden"));
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "-enc"));
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "-encodedcommand"));
    rule->AddCondition(std::move(cmdline));

    rule->AddAction("kill_process");
    engine.AddRule(std::move(rule));
}

void AddAutostartWriteRule(RuleEngine& engine) {
    auto rule = MakeRule<MatchRule>("autostart_write", Severity::kHigh);
    rule->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{
        EventType::kFileCreate, EventType::kFileWrite, EventType::kFileMove}));

    auto target = std::make_unique<AnyOf>();
    target->Add(std::make_unique<FieldContains>("path", "\\start menu\\programs\\startup\\"));
    target->Add(std::make_unique<FieldContains>("path", "/start menu/programs/startup/"));
    target->Add(std::make_unique<FieldContains>("to", "\\start menu\\programs\\startup\\"));
    target->Add(std::make_unique<FieldContains>("to", "/start menu/programs/startup/"));
    rule->AddCondition(std::move(target));

    rule->AddAction("quarantine_file");
    engine.AddRule(std::move(rule));
}

void AddRansomExtensionRule(RuleEngine& engine) {
    auto rule = MakeRule<MatchRule>("ransom_extension", Severity::kCritical);
    rule->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{
        EventType::kFileCreate, EventType::kFileWrite, EventType::kFileMove}));

    auto paths = std::make_unique<AnyOf>();
    paths->Add(std::make_unique<FieldEndsWith>("path", ".locked"));
    paths->Add(std::make_unique<FieldEndsWith>("to", ".locked"));
    rule->AddCondition(std::move(paths));

    rule->AddAction("quarantine_file");
    engine.AddRule(std::move(rule));
}
void AddScriptHostBeacon(RuleEngine& engine) {
    auto beacon = MakeRule<SequenceRule>("script_host_beacon",
                                                 Severity::kCritical, 10000);

    auto first = std::make_unique<AllOf>();

    first->Add(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kProcessStart}));

        
    auto image = std::make_unique<AnyOf>();
    image->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
    image->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
    first->Add(std::move(image));

    auto cmdline = std::make_unique<AnyOf>();
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "\\appdata\\local\\temp\\"));
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "/appdata/local/temp/"));
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "\\windows\\temp\\"));
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "/windows/temp/"));
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "%temp%"));
    cmdline->Add(std::make_unique<FieldContains>("cmdline", "%tmp%"));
    first->Add(std::move(cmdline));

    beacon->SetFirst(std::move(first));

    beacon->SetSecond(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kNetConnect}));

    beacon->AddAction("kill_process");
    engine.AddRule(std::move(beacon));
}
void AddScriptMassWriteRule(RuleEngine& engine) {
    auto mass_write = MakeRule<ThresholdRule<128>>(
        "ransomware_mass_write", Severity::kCritical,
        20,      // сколько событий
        10000,   // за сколько миллисекунд
        "pid");  // порог считается для каждого pid отдельно
    
    mass_write->SetCondition(MakeCondition(AllOfStatic(
    EventTypeIs({EventType::kFileCreate, EventType::kFileWrite,
                 EventType::kFileMove}),
    AnyOfStatic(FieldContains("path", "\\Documents\\"),
                FieldContains("path", "\\Desktop\\"),
                FieldContains("to", "\\Documents\\"),
                FieldContains("to", "\\Desktop\\")
                ))));
    mass_write->AddAction("kill_process");
    mass_write->AddAction("quarantine_file");


    engine.AddRule(std::move(mass_write));
}

void AddShellSpawnsScriptHost(RuleEngine& engine, const EntityModel* model) {
    auto shell_spawns_script_host = MakeRule<SequenceRule>(
        "shell_spawns_script_host", Severity::kHigh, 30000);
        
    auto first = std::make_unique<AllOf>();
    first->Add(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kProcessStart}));
    auto shell_image = std::make_unique<AnyOf>();
    shell_image->Add(std::make_unique<FieldEndsWith>("image", "cmd.exe"));
    shell_image->Add(std::make_unique<FieldEndsWith>("image", "powershell.exe"));
    shell_image->Add(std::make_unique<FieldEndsWith>("image", "pwsh.exe"));
    first->Add(std::move(shell_image));
    shell_spawns_script_host->SetFirst(std::move(first));

    auto second = std::make_unique<AllOf>();
    second->Add(std::make_unique<EventTypeIs>(
        std::vector<EventType>{EventType::kProcessStart}));
    auto image = std::make_unique<AnyOf>();
    image->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
    image->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
    second->Add(std::move(image));
    shell_spawns_script_host->SetSecond(std::move(second));

    shell_spawns_script_host->set_child_of_prev(true);
    shell_spawns_script_host->SetModel(model);
    shell_spawns_script_host->AddAction("kill_process");
    engine.AddRule(std::move(shell_spawns_script_host));
} 

void AddSpawnedFromDroppedFileRule(RuleEngine& engine, const EntityModel* model) {
    auto dropped = MakeRule<LambdaRule>("spawned_from_dropped_file", Severity::kHigh, [model](const Event& event){
        if (event.event_type() != EventType::kProcessStart) {
            return false;
        }
        const FileRecord* source = model->FindSpawnSource(event.pid());
        if (source != nullptr && source->writer_pid != "" && source->writer_pid != event.pid()) {
            return true;
        }
        return false;
    });
    dropped->AddAction("kill_process");
    dropped->AddAction("quarantine_file");
    engine.AddRule(std::move(dropped));
}

} // namespace

RuleEngine BuildAgentRules(const EntityModel* model) {
    RuleEngine engine;
    engine.SetModel(model);
    AddScriptHostFromTempRule(engine);
    AddLolbinDownloadRule(engine);
    AddHiddenPowershellRule(engine);
    AddAutostartWriteRule(engine);
    AddRansomExtensionRule(engine);
    AddScriptHostBeacon(engine);
    AddScriptMassWriteRule(engine);
    AddShellSpawnsScriptHost(engine, model);
    AddSpawnedFromDroppedFileRule(engine, model);
    return engine;
}

}  // namespace nano_edr