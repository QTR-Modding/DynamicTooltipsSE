#include "Settings.h"
#include "Modules.h"
#include "Utils.h"
#include "ClibUtil/detail/SimpleIni.h"

void Settings::Load() {
    CSimpleIniA ini;
    ini.SetUnicode();

    const auto loadRC = ini.LoadFile(INI::path.c_str());
    if (loadRC < 0) {
        logger::info("INI not found or failed to load (rc={}): {}", loadRC, INI::path);
    }

    Modules::modules.clear();

    for (auto i = 0; i < static_cast<int>(Modules::Modules::kTotal); ++i) {
        const auto module = static_cast<Modules::Modules>(i);
        const auto moduleName = ToString(module);

        ModuleFeatures features;

        // Read existing values (or defaults)
        {
            const auto key = "b" + moduleName;
            features.enabled = ini.GetBoolValue("Modules", key.c_str(), features.enabled);
        }
        {
            const auto key = "iColor" + moduleName;
            uint32_t c = Utils::ConvertColor(features.titleColor);
            c = static_cast<uint32_t>(ini.GetLongValue("Modules", key.c_str(), c));
            features.titleColor = Utils::ConvertColor(c);
        }

        features.getLore = Modules::GetLoreGetter(module);

        auto a_name = "quantDT" + moduleName;
        Modules::modules.emplace(module, Module{a_name, features});
    }

    // Other
    show_titles = ini.GetBoolValue("Other", "bShowTitles", show_titles);
    disallow_editorIDs = ini.GetBoolValue("Other", "bDisallowEditorIDs", disallow_editorIDs);
    show_spell_levels = ini.GetBoolValue("Other", "bShowSpellLevels", show_spell_levels);
    max_mod_names = ini.GetLongValue("Other", "iMaxModNames", max_mod_names);

    if (loadRC < 0) {
        Save();
    }
}

void Settings::Save() {
    CSimpleIniA ini;
    ini.SetUnicode();

    ini.LoadFile(INI::path.c_str());

    // Ensure folder exists
    try {
        const std::filesystem::path p{INI::path};
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }
    } catch (const std::exception& e) {
        logger::error("Failed to create directories for INI path '{}': {}", INI::path, e.what());
    }

    for (auto i = 0; i < static_cast<int>(Modules::Modules::kTotal); ++i) {
        const auto module = static_cast<Modules::Modules>(i);
        const auto moduleName = ToString(module);

        bool enabled = true;
        RE::NiColor color(0.8f, 0.8f, 0.2f);

        if (auto it = Modules::modules.find(module); it != Modules::modules.end()) {
            enabled = it->second.IsEnabled();
            color = it->second.GetColor();
        }

        // Modules
        ini.SetBoolValue("Modules", ("b" + moduleName).c_str(), enabled);
        ini.SetLongValue("Modules", ("iColor" + moduleName).c_str(), Utils::ConvertColor(color));
    }

    // Other
    ini.SetBoolValue("Other", "bShowTitles", show_titles);
    ini.SetBoolValue("Other", "bDisallowEditorIDs", disallow_editorIDs);
    ini.SetBoolValue("Other", "bShowSpellLevels", show_spell_levels);
    ini.SetLongValue("Other", "iMaxModNames", max_mod_names);

    const auto saveRC = ini.SaveFile(INI::path.c_str());
    if (saveRC < 0) {
        logger::error("SaveFile failed (rc={}): {}", saveRC, INI::path);
    } else {
        logger::info("INI saved: {}", INI::path);
    }
}