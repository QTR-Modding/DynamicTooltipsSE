#pragma once

namespace Utils {
    RE::BGSKeyword* MakeKeyword(const std::string& a_kw_name);
    RE::TESQuest* GetQuest(const RE::InventoryEntryData* a_entryData);
    RE::TESForm* GetOwner(const RE::InventoryEntryData* a_entryData);
    RE::InventoryEntryData* GetSelectedEntryInMenu();
    uint32_t ConvertColor(const RE::NiColor& a_color);
    RE::NiColor ConvertColor(uint32_t a_color);
    inline bool is_empty(const char* s) { return s == nullptr || s[0] == '\0'; }
    std::wstring utf8_to_wstring(std::string_view s);
    RE::StandardItemData* GetSelectedItemDataInMenu();
    RE::TESObjectREFR* GetOwnerOfItem(const RE::StandardItemData* a_itemdata);
}