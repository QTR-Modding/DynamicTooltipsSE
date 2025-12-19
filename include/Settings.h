#pragma once
#include <unordered_set>

using LoreGetter = std::function<std::string(RE::InventoryEntryData*)>;

struct SubModFeatures {
    bool enabled = true;
    RE::NiColor titleColor = RE::NiColor(0.8f, 0.8f, 0.2f);
    LoreGetter getLore;
};

struct SubMod {
    std::string_view GetTitle() const { return title; }
    std::string GetLore() const;
    void BuildLoreCache(const RE::TESObjectREFR::InventoryItemMap& a_inv);
    void ClearLoreCache();

    explicit SubMod(const std::string& a_name, const SubModFeatures& a_features);

    explicit operator bool() const { return features.enabled; }
    void Toggle(bool a_enable);

    RE::NiColor GetColor() const;
    void ChangeColor(const RE::NiColor& a_color);

private:
    using LoreCache = std::unordered_set<RE::TESBoundObject*>;
    LoreCache loreCache;
    RE::BGSKeyword* kw;
    std::string title;
    SubModFeatures features;
};


namespace Settings {
    inline std::string mod_name = "Dynamic Tooltips";
    inline bool disallow_editorIDs = false;
    inline bool show_titles = true;

    void Load();
    void Save();

    enum class Modules : uint8_t {
        quantDTWQ = 0,
        quantDTIO,
        kTotal
    };

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

inline std::wstring result_str;
extern "C" __declspec(dllexport) const wchar_t* OnDynamicTranslationRequest(std::string_view a_key);