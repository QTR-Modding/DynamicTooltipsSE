#include "Settings.h"
#include <unordered_set>
#include "Utils.h"
#include "ClibUtil/detail/SimpleIni.h"
#include "ClibUtil/editorID.hpp"

namespace {
    std::string ToString(const Settings::Modules a_module) {
        switch (a_module) {
            case Settings::Modules::WhoseQuest:
                return "WhoseQuest";
            case Settings::Modules::WhoseItem:
                return "WhoseItem";
            default:
                return "Unknown";
        }
    }
}

std::string SubMod::GetLore() const {
    if (const auto a_entry = Utils::GetSelectedEntryInMenu()) {
        return features.getLore(a_entry);
    }
    return "";
}

void SubMod::BuildLoreCache(const RE::TESObjectREFR::InventoryItemMap& a_inv) {
    for (const auto& [obj, entry] : a_inv) {
        if (entry.first <= 0 || !obj->GetPlayable() || obj->Is(RE::FormType::LeveledItem)) {
            continue;
        }
        RE::BGSKeywordForm* a_kw_form;
        if (const auto ammo = obj->As<RE::TESAmmo>()) {
            a_kw_form = ammo->AsKeywordForm();
        } else {
            a_kw_form = obj->As<RE::BGSKeywordForm>();
        }
        if (!a_kw_form) {
            continue;
        }

        if (!loreCache.contains(obj)) {
            a_kw_form->AddKeyword(kw);
            loreCache.insert(obj);
        }
    }
}

void SubMod::ClearLoreCache() {
    for (const auto a_obj : loreCache) {
        RE::BGSKeywordForm* a_kw_form;
        if (const auto ammo = a_obj->As<RE::TESAmmo>()) {
            a_kw_form = ammo->AsKeywordForm();
        } else {
            a_kw_form = a_obj->As<RE::BGSKeywordForm>();
        }
        if (a_kw_form) {
            a_kw_form->RemoveKeyword(kw);
        }
    }
    loreCache.clear();
}

SubMod::SubMod(const std::string& a_name, const SubModFeatures& a_features) : features(a_features) {
    if (!SKSE::Translation::Translate("$" + a_name + "Title", title)) {
        logger::error("Failed to translate title for sub-mod '{}'", a_name);
    }
    if (kw = Utils::MakeKeyword("LoreBox_" + a_name); !kw) {
        logger::error("Failed to find keyword for sub-mod '{}'", a_name);
    }
}

void SubMod::Toggle(const bool a_enable) {
    features.enabled = a_enable;
    if (!features.enabled) {
        ClearLoreCache();
    }
}

RE::NiColor SubMod::GetColor() const { return features.titleColor; }

void SubMod::ChangeColor(const RE::NiColor& a_color) { features.titleColor = a_color; }

std::string SubMod::GetKeywordName() const { return kw->GetFormEditorID(); }

void Settings::Load() {
    CSimpleIniA ini;
    ini.SetUnicode();

    const auto loadRC = ini.LoadFile(INI::path.c_str());
    if (loadRC < 0) {
        logger::info("INI not found or failed to load (rc={}): {}", loadRC, INI::path);
    }

    subMods.clear();

    for (auto i = 0; i < static_cast<int>(Modules::kTotal); ++i) {
        const auto module = static_cast<Modules>(i);
        const auto moduleName = ToString(module);

        SubModFeatures features;

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

        features.getLore = LoreGetters::GetLoreGetter(module);

        auto a_name = "quantDT" + moduleName;
        subMods.emplace(module, SubMod{a_name, features});
    }

    // Other
    show_titles = ini.GetBoolValue("Other", "bShowTitles", show_titles);
    disallow_editorIDs = ini.GetBoolValue("Other", "bDisallowEditorIDs", disallow_editorIDs);

    // Only force-create/write if missing/failed load; otherwise you may overwrite user edits every startup.
    if (loadRC < 0) {
        Save();
    }
}

void Settings::Save() {
    CSimpleIniA ini;
    ini.SetUnicode();

    // Optional: load first to preserve unrelated sections/keys/comments
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

    for (auto i = 0; i < static_cast<int>(Modules::kTotal); ++i) {
        const auto module = static_cast<Modules>(i);
        const auto moduleName = ToString(module);

        // Pull values from your live subMods if present; otherwise write sensible defaults.
        bool enabled = true;
        RE::NiColor color(0.8f, 0.8f, 0.2f);

        if (auto it = subMods.find(module); it != subMods.end()) {
            enabled = static_cast<bool>(it->second);
            color = it->second.GetColor();
        }

        // Modules
        ini.SetBoolValue("Modules", ("b" + moduleName).c_str(), enabled);
        ini.SetLongValue("Modules", ("iColor" + moduleName).c_str(), Utils::ConvertColor(color));
        // Other
        ini.SetBoolValue("Other", "bShowTitles", show_titles);
        ini.SetBoolValue("Other", "bDisallowEditorIDs", disallow_editorIDs);
    }

    const auto saveRC = ini.SaveFile(INI::path.c_str());
    if (saveRC < 0) {
        logger::error("SaveFile failed (rc={}): {}", saveRC, INI::path);
    } else {
        logger::info("INI saved: {}", INI::path);
    }
}


LoreGetter Settings::LoreGetters::GetLoreGetter(Modules a_module) {
    switch (a_module) {
        case Modules::WhoseQuest:
            return GetLoreWQ;
        case Modules::WhoseItem:
            return GetLoreIO;
        default:
            return {};
    }
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
std::string Settings::LoreGetters::GetLoreWQ(RE::InventoryEntryData* a_entryData) {
    if (const auto a_quest = Utils::GetQuest(a_entryData)) {
        std::string a_name;
        if (a_name = a_quest->GetFullName(); !a_name.empty()) {
            return a_name;
        }
        if (a_name = a_quest->GetName(); !a_name.empty()) {
            return a_name;
        }
        if (!disallow_editorIDs) {
            if (a_name = clib_util::editorID::get_editorID(a_quest); !a_name.empty()) {
                return a_name;
            }
        }
    }
    return "";
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
std::string Settings::LoreGetters::GetLoreIO(RE::InventoryEntryData* a_entryData) {
    if (const auto a_owner = Utils::GetOwner(a_entryData)) {
        if (const auto a_fullnameform = a_owner->As<RE::TESFullName>()) {
            if (const auto a_name = a_fullnameform->GetFullName(); !Utils::is_empty(a_name)) {
                return a_name;
            }
        }
        if (const auto a_name = a_owner->GetName(); !Utils::is_empty(a_name)) {
            return a_name;
        }
        if (!disallow_editorIDs) {
            if (const auto a_name = clib_util::editorID::get_editorID(a_owner); !a_name.empty()) {
                return a_name;
            }
        }
    }
    return "";
}

// ReSharper disable once CppParameterMayBeConst
const wchar_t* OnDynamicTranslationRequest(std::string_view a_key) {
    result_str.clear();

    for (auto& a_submod : Settings::subMods | std::views::values) {
        if (a_key == a_submod.GetKeywordName()) {
            if (const auto lore = a_submod.GetLore(); !lore.empty()) {
                std::string title;
                if (Settings::show_titles) {
                    title = a_submod.GetTitle();
                }
                if (!title.empty()) {
                    // add color to the title
                    const auto packed = Utils::ConvertColor(a_submod.GetColor());
                    const uint32_t r = packed & 0xFF;
                    const uint32_t g = (packed >> 8) & 0xFF;
                    const uint32_t b = (packed >> 16) & 0xFF;

                    const uint32_t rgb = (r << 16) | (g << 8) | b;  // 0xRRGGBB
                    const auto hex = fmt::format("{:06X}", rgb);
                    const auto titleHtml = fmt::format("<font color=\"#{}\">{}</font>", hex, title);
                    result_str = Utils::utf8_to_wstring(titleHtml);
                    result_str += L"\n";
                }
                result_str += Utils::utf8_to_wstring(lore);
            }
            break;
        }
    }

    return result_str.c_str();
}