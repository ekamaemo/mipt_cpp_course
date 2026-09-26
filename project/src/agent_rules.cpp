#include<agent_rules.h>
#include "fields.h"

namespace nano_edr {
    namespace {
    bool ImageEndsWith(const Event& event, const std::string& suffix) {
        const std::string image = NormalizePath(GetRequiredField(event, "image"));
        const std::string normal_suf = NormalizePath(suffix);
        if (normal_suf.size() > image.size()) {
            return false;
        }
        return image.ends_with(normal_suf);
    }

    bool IsFileType(const Event& event){
        return event.type == "file_create"
            || event.type == "file_write"
            || event.type == "file_move";
    }

    const std::string* FileRulePath(const Event& event){
        return event.type == "file_move"? FindField(event, "to") : FindField(event, "path");
    }

    bool IsScriptHostFromTemp(const Event& event){
        if (!IsProcessStart(event)){
            return false;
        }

        if (!ImageEndsWith(event, "wscript.exe") && !ImageEndsWith(event, "cscript.exe")){
            return false;
        }

        return CommandLineContains(event, "\\appdata\\local\\temp") || CommandLineContains(event, "\\windows\\temp");

    }

    bool IsLolbinDownload(const Event& event){
        if (!IsProcessStart(event)){
            return false;
        }
        if (!ImageEndsWith(event, "certutil.exe") && !ImageEndsWith(event, "bitsadmin.exe")){
            return false;
        }
        
        return CommandLineContains(event, "urlcache") 
                || CommandLineContains(event, "transfer") 
                || CommandLineContains(event, "http:") 
                || CommandLineContains(event, "https:");
    }


    bool IsHiddenPowershell(const Event& event){
        if (!IsProcessStart(event)){
            return false;
        }
        if (!ImageEndsWith(event, "powershell.exe")
            && !ImageEndsWith(event, "pwsh.exe")) {
            return false;
        }
        return CommandLineContains(event, "-w hidden")
            || CommandLineContains(event, "-windowstyle hidden")
            || CommandLineContains(event, "-enc")
            || CommandLineContains(event, "-encodedcommand");

    }

    bool IsAutostartWrite(const Event& event){
        if (!IsFileType(event)){
            return false;
        }

        const std::string* path = FileRulePath(event);
        if (path == nullptr){
            return false;
        }
        const std::string normal_path = NormalizePath(*path);
        return normal_path.find("\\startmenu\\programs\\startup\\") != std::string::npos;

    }

    bool IsRansomExtension(const Event& event){
        if (!IsFileType(event)){
            return false;
        }
        
        const std::string* path = FileRulePath(event);
        if (path == nullptr){
            return false;
        }
        const std::string normal_path = NormalizePath(*path);
        return normal_path.ends_with(".locked");
    }
} // конец namespace с доп функциями

constexpr Rule kRules[] {
    {"script_host_from_temp", IsScriptHostFromTemp, Severity::kHigh},
    {"lolbin_download",       IsLolbinDownload,      Severity::kHigh},
    {"hidden_powershell",     IsHiddenPowershell,    Severity::kMedium},
    {"autostart_write",       IsAutostartWrite,      Severity::kHigh},
    {"ransom_extension",      IsRansomExtension,     Severity::kCritical},
};

const Rule* AgentRules(){
    return kRules;
}


size_t AgentRuleCount() {
    return sizeof(kRules) / sizeof(kRules[0]);
}
}