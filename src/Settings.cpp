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

RE::NiColorA SubMod::GetColor() const { return features.titleColor; }

void SubMod::ChangeColor(const RE::NiColorA& a_color) { features.titleColor = a_color; }

void SubMod::BuildLore(const RE::TESObjectREFR::InventoryItemMap& a_inv, LoreCache& a_cache) const {
    std::unordered_set<RE::FormID> currentLore;
    for (const auto& obj : a_cache | std::views::keys) {
        currentLore.insert(obj->GetFormID());
    }
    for (const auto& [obj, entry] : a_inv) {
        if (entry.first <= 0 || !obj->GetPlayable() || obj->Is(RE::FormType::LeveledItem)) {
            continue;
        }
        const auto a_kw_form = obj->As<RE::BGSKeywordForm>();
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
    loreCachePlayer.clear();
    loreCacheContainer.clear();
}

void Settings::Load() {
    CSimpleIniA ini;
    ini.SetUnicode();

    if (const SI_Error rc = ini.LoadFile(INI::path.c_str()); rc < 0) {
        ini.SaveFile(INI::path.c_str());
    }

    for (auto i = 0; i < static_cast<int>(Modules::kTotal); ++i) {
        const auto a_module = static_cast<Modules>(i);
        auto a_module_name = ToString(a_module);
        if (const auto a_kw = Utils::MakeKeyword(a_module_name)) {
            SubModFeatures features;

            auto a_name = "b" + a_module_name;
            features.enabled = ini.GetBoolValue("Modules", a_name.c_str(), features.enabled);

            a_name = "iColor" + a_module_name;
            uint32_t a_titleColor = Utils::ConvertColor(features.titleColor);
            a_titleColor = static_cast<uint32_t>(ini.GetLongValue("Modules", a_name.c_str(), a_titleColor));
            features.titleColor = Utils::ConvertColor(a_titleColor);

            features.getLore = LoreGetters::GetLoreGetter(a_module);

            subMods.emplace(a_module, SubMod{a_kw, features});
        } else {
            logger::error("Failed to find keyword for sub-mod '{}'", a_module_name);
        }
    }

    ini.SaveFile(INI::path.c_str());
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
        if (a_name = clib_util::editorID::get_editorID(a_quest); !a_name.empty()) {
            return a_name;
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
        if (const auto a_name = clib_util::editorID::get_editorID(a_owner); !a_name.empty()) {
            return a_name;
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
            translated = Utils::utf8_to_wstring(subModIt->second.GetLore());
        }
    }
    return translated.c_str();
}