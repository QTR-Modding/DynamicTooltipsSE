#pragma once

namespace LoreBox {
    std::string GetLoreWQ(const RE::InventoryEntryData* a_entryData);
    std::string GetLoreIO(const RE::InventoryEntryData* a_entryData);
    std::string GetLoreSPBMGCK(RE::InventoryEntryData* a_entryData);
    std::string GetLoreWhichMods(const RE::InventoryEntryData* a_entryData);
}