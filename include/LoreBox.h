#pragma once

namespace LoreBox::LoreGetters {
    std::string GetLoreWQ(const RE::InventoryEntryData* a_entryData);
    std::string GetLoreIO(const RE::InventoryEntryData* a_entryData);
    std::string GetLoreSPBMGCK(RE::InventoryEntryData* a_entryData);
}