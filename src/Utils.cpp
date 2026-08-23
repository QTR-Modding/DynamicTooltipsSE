#include "Utils.h"

namespace {
    template <typename T>
    RE::InventoryEntryData* GetSelectedEntryInMenuHelper() {
        if (const auto a_menu = RE::UI::GetSingleton()->GetMenu<T>()) {
            if (const auto a_itemList = a_menu->GetRuntimeData().itemList) {
                if (const auto item = a_itemList->GetSelectedItem()) {
                    return item->data.objDesc;
                }
            }
        }
        return nullptr;
    }
}

RE::BGSKeyword* Utils::MakeKeyword(const std::string& a_kw_name) {
    return RE::BGSKeyword::CreateKeyword(a_kw_name);
}

RE::TESQuest* Utils::GetQuest(const RE::InventoryEntryData* a_entryData) {
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

RE::TESForm* Utils::GetOwner(const RE::InventoryEntryData* a_entryData) {
    RE::TESForm* owner = nullptr;
    if (a_entryData && a_entryData->extraLists) {
        for (const auto& xList : *a_entryData->extraLists) {
            if (const auto xOwner = xList->GetByType<RE::ExtraOwnership>()) {
                if (const auto a_owner = xOwner->owner) {
                    owner = a_owner;
                    break;
                }
            }
        }
    }
    return owner;
}

std::vector<RE::TESFile*> Utils::GetOwningMods(const RE::InventoryEntryData* a_entryData) {
    std::vector<RE::TESFile*> mods;
    #undef GetObject
    const auto a_obj = a_entryData->GetObject();
    if (const auto a_mods = a_obj->sourceFiles.array) {
        for (const auto& mod : *a_mods) {
            if (mod) mods.push_back(mod);
        }
    }
    return mods;
}

RE::TESBoundObject* Utils::GetSelectedCraftingObject() {
    const auto menu = RE::UI::GetSingleton()->GetMenu<RE::CraftingMenu>();
    if (!menu) {
        return nullptr;
    }

    const auto subMenu = menu->GetCraftingSubMenu();
    if (const auto constructibleMenu =
        skyrim_cast<RE::CraftingSubMenus::ConstructibleObjectMenu*>(subMenu)) {
        if (constructibleMenu->currentIndex >= constructibleMenu->recipes.size()) {
            return nullptr;
        }

        const auto constructibleObject =
            constructibleMenu->recipes[constructibleMenu->currentIndex].constructibleObject;
        return constructibleObject && constructibleObject->createdItem
                   ? constructibleObject->createdItem->As<RE::TESBoundObject>()
                   : nullptr;
    }

    return nullptr;
}

RE::TESBoundObject* Utils::GetSelectedMagicObject() {
    if (const auto menu = RE::UI::GetSingleton()->GetMenu<RE::MagicMenu>()) {
        if (const auto itemList = menu->GetRuntimeData().itemList) {
            if (const auto item = itemList->GetSelectedItem()) {
                return item->data.baseForm ? item->data.baseForm->As<RE::TESBoundObject>() : nullptr;
            }
        }
    }
    return nullptr;
}

RE::InventoryEntryData* Utils::GetSelectedEntryInMenu() {
    if (const auto a_item = GetSelectedEntryInMenuHelper<RE::InventoryMenu>()) {
        return a_item;
    }
    if (const auto a_item = GetSelectedEntryInMenuHelper<RE::ContainerMenu>()) {
        return a_item;
    }
    if (const auto a_item = GetSelectedEntryInMenuHelper<RE::BarterMenu>()) {
        return a_item;
    }
    if (const auto a_item = GetSelectedEntryInMenuHelper<RE::GiftMenu>()) {
        return a_item;
    }
    return nullptr;
}

uint32_t Utils::ConvertColor(const RE::NiColor& a_color) {
    const uint8_t r = static_cast<uint8_t>(a_color.red * 255.0f);
    const uint8_t g = static_cast<uint8_t>(a_color.green * 255.0f);
    const uint8_t b = static_cast<uint8_t>(a_color.blue * 255.0f);
    return (static_cast<uint32_t>(b) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(r);
}

RE::NiColor Utils::ConvertColor(const uint32_t a_color) {
    const float r = static_cast<float>(a_color & 0xFF) / 255.0f;
    const float g = static_cast<float>((a_color >> 8) & 0xFF) / 255.0f;
    const float b = static_cast<float>((a_color >> 16) & 0xFF) / 255.0f;
    return {r, g, b};
}

std::wstring Utils::utf8_to_wstring(const std::string_view s) {
    if (s.empty()) return {};

    const int needed = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);

    std::wstring out(needed, L'\0');

    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), out.data(), needed);

    return out;
}
