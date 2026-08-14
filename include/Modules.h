#pragma once
#include <unordered_set>
#include "LoreBox.h"

using LoreGetter = std::function<std::string(RE::InventoryEntryData*)>;

struct ModuleFeatures {
    RE::NiColor titleColor = RE::NiColor(67.f / 255.f, 222.f / 255.f, 16.f / 255.f);
    LoreGetter getLore;
    bool enabled = true;
};

struct Module {
    std::string_view GetTitle() const { return title; }
    std::string GetLore() const;
    void BuildLoreCache(RE::TESBoundObject* a_obj);
    void BuildSpellLoreCache(RE::SpellItem* a_spell);
    void BuildLoreCache(const RE::TESObjectREFR::InventoryItemMap& a_inv);
    void BuildLoreCache(const RE::ItemList* a_itemList);
    void ClearLoreCache();

    explicit Module(const std::string& a_name, const ModuleFeatures& a_features);

    bool IsEnabled() const { return features.enabled; }
    void Toggle(bool a_enable);

    RE::NiColor GetColor() const;
    void ChangeColor(const RE::NiColor& a_color);
    std::string GetKeywordName() const;

private:
    using LoreCache = std::unordered_set<RE::TESBoundObject*>;
    LoreCache loreCache;
    std::unordered_set<RE::EffectSetting*> spellLoreCache;
    RE::BGSKeyword* kw;
    std::string title;
    ModuleFeatures features;
};

namespace Modules {
    enum class Modules : uint8_t { WhoseQuest = 0, WhoseItem, SPBMGCK, WhichMods, kTotal };

    inline std::string ToString(const Modules a_module) {
        switch (a_module) {
            case Modules::WhoseQuest:
                return "WhoseQuest";
            case Modules::WhoseItem:
                return "WhoseItem";
            case Modules::SPBMGCK:
                return "SPBMGCK";
            case Modules::WhichMods:
                return "WhichMods";
            default:
                return "Unknown";
        }
    }

    inline LoreGetter GetLoreGetter(const Modules a_module) {
        switch (a_module) {
            case Modules::WhoseQuest:
                return LoreBox::GetLoreWQ;
            case Modules::WhoseItem:
                return LoreBox::GetLoreIO;
            case Modules::SPBMGCK:
                return LoreBox::GetLoreSPBMGCK;
            case Modules::WhichMods:
                return LoreBox::GetLoreWhichMods;
            default:
                return {};
        }
    }

    inline std::unordered_map<Modules, Module> modules;
}
