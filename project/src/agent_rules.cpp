#include <agent_rules.h>
#include <fields.h>

namespace nano_edr {
std::vector<MatchRule> BuildAgentRules(){
    std::vector<MatchRule> rules;
    {
        MatchRule by_path("ransom_extension", Severity::kCritical);
        by_path.AddCondition(Condition::Equals("type", {"file_create", "file_write"}));
        by_path.AddCondition(Condition::EndsWith("path", {".locked"}));
        by_path.AddAction("quarantine_file");
        rules.push_back(by_path);
    }

    {
        MatchRule by_to("ransom_extension", Severity::kCritical);
        by_to.AddCondition(Condition::Equals("type", {"file_move"}));
        by_to.AddCondition(Condition::EndsWith("to", {".locked"}));
        by_to.AddAction("quarantine_file");
        rules.push_back(by_to);
    }

    {
        MatchRule rule("script_host_from_temp", Severity::kHigh);
        rule.AddCondition(Condition::Equals("type", {"process_start"}));
        rule.AddCondition(
            Condition::EndsWith("image", {"wscript.exe", "cscript.exe"}));
        rule.AddCondition(Condition::Contains("cmdline", {
            "\\appdata\\local\\temp\\", "/appdata/local/temp/",
            "\\windows\\temp\\", "/windows/temp/",
            "%temp%", "%tmp%"}));
        rule.AddAction("kill_process");
        rules.push_back(rule);
    }
    {
        MatchRule rule("lolbin_download", Severity::kHigh);
        rule.AddCondition(Condition::Equals("type", {"process_start"}));
        rule.AddCondition(Condition::EndsWith("image", {"certutil.exe", "bitsadmin.exe"}));
        rule.AddCondition(Condition::Contains("cmdline", {"urlcache", "transfer", "http:", "https:"}));
        rule.AddAction("kill_process");
        rules.push_back(rule);
    }

    {
        MatchRule rule("hidden_powershell", Severity::kMedium);
        rule.AddCondition(Condition::Equals("type", {"process_start"}));
        rule.AddCondition(Condition::EndsWith("image", {"powershell.exe", "pwsh.exe"}));
        rule.AddCondition(Condition::Contains("cmdline", {"-w hidden", "-windowstyle hidden", "-enc", "-encodedcommand"}));
        rule.AddAction("kill_process");
        rules.push_back(rule);
    }

    // autostart_write — по path (file_create / file_write)
    {
        MatchRule rule("autostart_write", Severity::kHigh);
        rule.AddCondition(Condition::Equals("type", {"file_create", "file_write"}));
        rule.AddCondition(Condition::Contains("path", {
            "\\start menu\\programs\\startup\\",
            "/start menu/programs/startup/"}));
        rule.AddAction("quarantine_file");
        rules.push_back(rule);
    }

    // autostart_write — по to (file_move)
    {
        MatchRule rule("autostart_write", Severity::kHigh);
        rule.AddCondition(Condition::Equals("type", {"file_move"}));
        rule.AddCondition(Condition::Contains("to", {
            "\\start menu\\programs\\startup\\",
            "/start menu/programs/startup/"}));
        rule.AddAction("quarantine_file");
        rules.push_back(rule);
    }
    return rules;
}
}