#pragma once

namespace Utils {
    inline RE::TESQuest* GetQuest(const RE::InventoryEntryData* a_entryData) {
        RE::TESQuest* quest = nullptr;
        if (a_entryData && a_entryData->extraLists) {
            for (const auto& xList : *a_entryData->extraLists) {
                if (const auto xAliasInstArr = xList->GetByType<RE::ExtraAliasInstanceArray>()) {
                    for (const auto& instance : xAliasInstArr->aliases) {
                        if (instance->quest && instance->alias && instance->alias->IsQuestObject()) {
                            quest = instance->quest;
                            break;
                        }
                    }
                }
                if (quest) {
                    break;
                }
            }
        }
        return quest;
    }
}