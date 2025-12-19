#pragma once

using LoreGetter = std::function<std::string(RE::InventoryEntryData*)>;

struct SubModFeatures {
    bool enabled = true;
    RE::NiColorA titleColor = RE::NiColorA(0.8f, 0.8f, 0.2f, 1.0f);
    LoreGetter getLore;
};

struct SubMod {
    std::string_view GetTitle() const { return title; }
    std::string GetLore();
    void BuildLorePlayer(const RE::TESObjectREFR::InventoryItemMap& a_inv);
    void BuildLoreContainer(const RE::TESObjectREFR::InventoryItemMap& a_inv);

    explicit SubMod(RE::BGSKeyword* a_kw, const SubModFeatures& a_features) : kw(a_kw), features(a_features) {
        if (!SKSE::Translation::Translate("$" + std::string(kw->GetFormEditorID()), title)) {
            logger::error("Failed to translate title for sub-mod '{}'", kw->GetFormEditorID());
        }
    }

    explicit operator bool() const { return features.enabled; }
    void Toggle(bool a_enable);

    RE::NiColorA GetColor() const;
    void ChangeColor(const RE::NiColorA& a_color);

private:
    using LoreCache = std::unordered_map<RE::TESBoundObject*, std::string>;
    void BuildLore(const RE::TESObjectREFR::InventoryItemMap& a_inv, LoreCache& a_cache) const;
    void Clear();
    LoreCache loreCachePlayer;
    LoreCache loreCacheContainer;
    RE::BGSKeyword* kw;
    std::string title;
    SubModFeatures features;
};


namespace Settings {
    inline std::string mod_name = "Dynamic Tooltips";

    void Load();

    enum class Modules : uint8_t {
        LoreBox_quantDTWQ = 0,
        LoreBox_quantDTIO,
        kTotal
    };

    Modules StringToModule(std::string_view a_str);

    namespace INI {
        inline std::string path = "Data\\SKSE\\Plugins\\DynamicTooltips\\DynamicTooltips.ini";
    }

    inline std::unordered_map<Modules, SubMod> subMods;

    namespace LoreGetters {
        LoreGetter GetLoreGetter(Modules a_module);
        std::string GetLoreWQ(RE::InventoryEntryData* a_entryData);
        std::string GetLoreIO(RE::InventoryEntryData* a_entryData);
    }
}

extern "C" __declspec(dllexport) const wchar_t* OnDynamicTranslationRequest(std::string_view a_key);