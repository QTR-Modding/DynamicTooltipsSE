#include "Settings.h"
#include <unordered_set>
#include "Utils.h"
#include "ClibUtil/detail/SimpleIni.h"
#include "ClibUtil/editorID.hpp"

namespace {
    std::string ToString(const Settings::Modules a_module) {
        switch (a_module) {
            case Settings::Modules::LoreBox_quantDTWQ:
                return "LoreBox_quantDTWQ";
            case Settings::Modules::LoreBox_quantDTIO:
                return "LoreBox_quantDTIO";
            default:
                return "Unknown";
        }
    }
}

std::string SubMod::GetLore() {
    if (const auto item_data = Utils::GetSelectedItemDataInMenu()) {
        const auto owner = Utils::GetOwnerOfItem(item_data);
        #undef GetObject
        if (const auto a_bound = item_data->objDesc->GetObject()) {
            LoreCache& a_cache = owner->IsPlayerRef() ? loreCachePlayer : loreCacheContainer;
            if (const auto cacheIt = a_cache.find(a_bound); cacheIt != a_cache.end()) {
                return cacheIt->second;
            }
        }
    }
    return "";
}

void SubMod::BuildLorePlayer(const RE::TESObjectREFR::InventoryItemMap& a_inv) { BuildLore(a_inv, loreCachePlayer); }

void SubMod::BuildLoreContainer(const RE::TESObjectREFR::InventoryItemMap& a_inv) {
    BuildLore(a_inv, loreCacheContainer);
}

void SubMod::Toggle(const bool a_enable) {
    features.enabled = a_enable;
    if (!features.enabled) {
        Clear();
    }
}

RE::NiColor SubMod::GetColor() const { return features.titleColor; }

void SubMod::ChangeColor(const RE::NiColor& a_color) { features.titleColor = a_color; }

void SubMod::BuildLore(const RE::TESObjectREFR::InventoryItemMap& a_inv, LoreCache& a_cache) const {
    std::unordered_set<RE::FormID> currentLore;
    for (const auto& obj : a_cache | std::views::keys) {
        currentLore.insert(obj->GetFormID());
    }
    for (const auto& [obj, entry] : a_inv) {
        if (entry.first <= 0 || !obj->GetPlayable() || obj->Is(RE::FormType::LeveledItem)) {
            continue;
        }
        RE::BGSKeywordForm* a_kw_form = nullptr;
        if (const auto ammo = obj->As<RE::TESAmmo>()) {
            a_kw_form = ammo->AsKeywordForm();
        } else {
            a_kw_form = obj->As<RE::BGSKeywordForm>();
        }
        if (!a_kw_form) {
            continue;
        }

        const auto a_inv_data = entry.second.get();
        if (auto a_lore = features.getLore(a_inv_data); !a_lore.empty()) {
            a_cache[obj] = a_lore;
            if (auto it = currentLore.find(obj->GetFormID()); it != currentLore.end()) {
                currentLore.erase(it);
            } else {
                a_kw_form->AddKeyword(kw);
            }
        }
    }
    for (const auto& a_formid : currentLore) {
        if (const auto a_obj = RE::TESForm::LookupByID<RE::TESBoundObject>(a_formid)) {
            if (const auto a_kw_form = a_obj->As<RE::BGSKeywordForm>()) {
                a_kw_form->RemoveKeyword(kw);
            }
            a_cache.erase(a_obj);
        }
    }
}

void SubMod::Clear() {
    for (const auto a_obj : loreCachePlayer | std::views::keys) {
        if (const auto a_kw_form = a_obj->As<RE::BGSKeywordForm>()) {
            a_kw_form->RemoveKeyword(kw);
        }
    }
    loreCachePlayer.clear();
    for (const auto a_obj : loreCacheContainer | std::views::keys) {
        if (const auto a_kw_form = a_obj->As<RE::BGSKeywordForm>()) {
            a_kw_form->RemoveKeyword(kw);
        }
    }
    loreCacheContainer.clear();
}

void Settings::Load() {
    CSimpleIniA ini;
    ini.SetUnicode();

    const auto loadRC = ini.LoadFile(INI::path.c_str());
    if (loadRC < 0) {
        logger::info("INI not found or failed to load (rc={}): {}", loadRC, INI::path);
        // We'll still proceed; defaults will be used and then written out.
    }

    subMods.clear();

    for (auto i = 0; i < static_cast<int>(Modules::kTotal); ++i) {
        const auto module = static_cast<Modules>(i);
        const auto moduleName = ToString(module);

        if (const auto kw = Utils::MakeKeyword(moduleName)) {
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

            subMods.emplace(module, SubMod{kw, features});
        } else {
            logger::error("Failed to find keyword for sub-mod '{}'", moduleName);
        }
    }

    // Other
    disallow_editorIDs = ini.GetBoolValue("Other", "bDisallowEditorIDs", disallow_editorIDs);

    // Only force-create/write if missing/failed load; otherwise you may overwrite user edits every startup.
    if (loadRC < 0) {
        Save();
    }

    for (auto& a_subMod : subMods | std::views::values) {
        a_subMod.Clear();
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
        ini.SetBoolValue("Other", "bDisallowEditorIDs", disallow_editorIDs);
    }

    const auto saveRC = ini.SaveFile(INI::path.c_str());
    if (saveRC < 0) {
        logger::error("SaveFile failed (rc={}): {}", saveRC, INI::path);
    } else {
        logger::info("INI saved: {}", INI::path);
    }
}


Settings::Modules Settings::StringToModule(const std::string_view a_str) {
    for (auto i = 0; i < static_cast<int>(Modules::kTotal); ++i) {
        if (const auto a_module = static_cast<Modules>(i); a_str == ToString(a_module)) {
            return a_module;
        }
    }
    return Modules::kTotal;
}

LoreGetter Settings::LoreGetters::GetLoreGetter(Modules a_module) {
    switch (a_module) {
        case Modules::LoreBox_quantDTWQ:
            return GetLoreWQ;
        case Modules::LoreBox_quantDTIO:
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
        logger::info("Found owner formID: {:08X}", a_owner->GetFormID());
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
    static std::wstring translated;
    translated.clear();

    if (const auto a_module = Settings::StringToModule(a_key); a_module < Settings::Modules::kTotal) {
        if (const auto subModIt = Settings::subMods.find(a_module); subModIt != Settings::subMods.end()) {
            if (const auto lore = subModIt->second.GetLore(); !lore.empty()) {
                std::string title;
                title = subModIt->second.GetTitle();
                if (!title.empty()) {
                    // add color to the title
                    const auto packed = Utils::ConvertColor(subModIt->second.GetColor());
                    const uint32_t r = packed & 0xFF;
                    const uint32_t g = (packed >> 8) & 0xFF;
                    const uint32_t b = (packed >> 16) & 0xFF;

                    const uint32_t rgb = (r << 16) | (g << 8) | b; // 0xRRGGBB
                    const auto hex = fmt::format("{:06X}", rgb);
                    const auto titleHtml = fmt::format("<font color=\"#{}\">{}</font>", hex, title);
                    translated = Utils::utf8_to_wstring(titleHtml);
                }
                // now add the actual text under the title
                translated += L"\n";
                translated += Utils::utf8_to_wstring(lore);
            }
        }
    }
    return translated.c_str();
}