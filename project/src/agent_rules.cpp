#include "agent_rules.h"
#include "rule_engine.h"

#include <memory>
#include <utility>
#include <vector>

namespace nano_edr {
namespace {

void AddScriptHostFromTempRule(RuleEngine& engine) {
    auto rule = std::make_unique<MatchRule>("script_host_from_temp", Severity::kHigh);
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
    auto rule = std::make_unique<MatchRule>("lolbin_download", Severity::kHigh);
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
    auto rule = std::make_unique<MatchRule>("hidden_powershell", Severity::kMedium);
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
    auto rule = std::make_unique<MatchRule>("autostart_write", Severity::kHigh);
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
    auto rule = std::make_unique<MatchRule>("ransom_extension", Severity::kCritical);
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
    auto beacon = std::make_unique<SequenceRule>("script_host_beacon",
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
}  // namespace

RuleEngine BuildAgentRules() {
    RuleEngine engine;
    AddScriptHostFromTempRule(engine);
    AddLolbinDownloadRule(engine);
    AddHiddenPowershellRule(engine);
    AddAutostartWriteRule(engine);
    AddRansomExtensionRule(engine);
    AddScriptHostBeacon(engine);
    return engine;
}

}  // namespace nano_edr