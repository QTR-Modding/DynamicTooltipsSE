#include "Utils.h"

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

RE::InventoryEntryData* Utils::GetSelectedEntryInMenu() {
    if (const auto ui = RE::UI::GetSingleton()) {
        if (const auto menu_c = ui->GetMenu<RE::ContainerMenu>()) {
            if (const auto a_itemList = menu_c->GetRuntimeData().itemList) {
                if (const auto item = a_itemList->GetSelectedItem()) {
                    return item->data.objDesc;
                }
            }
        } else if (const auto menu_i = ui->GetMenu<RE::InventoryMenu>()) {
            if (const auto a_itemList = menu_i->GetRuntimeData().itemList) {
                if (const auto item = a_itemList->GetSelectedItem()) {
                    return item->data.objDesc;
                }
            }
        } else if (const auto menu_f = ui->GetMenu<RE::FavoritesMenu>()) {
            RE::GFxValue selectedIndex;
            const auto& runtime_data = menu_f->GetRuntimeData();
            if (runtime_data.root.GetMember("selectedIndex", &selectedIndex) && selectedIndex.IsNumber()) {
                const std::int32_t selected_index = static_cast<std::int32_t>(selectedIndex.GetNumber());
                const auto& items = runtime_data.favorites;
                if (selected_index >= 0 && static_cast<uint32_t>(selected_index) < items.size()) {
                    return items[selected_index].entryData;
                }
            }
        }
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

template <typename MenuType>
RE::StandardItemData* GetSelectedItemData() {
    if (const auto ui = RE::UI::GetSingleton()) {
        if (const auto menu = ui->GetMenu<MenuType>()) {
            if (RE::ItemList* a_itemList = menu->GetRuntimeData().itemList) {
                if (auto* item = a_itemList->GetSelectedItem()) {
                    return &item->data;
                }
            }
        }
    }
    return nullptr;
}

RE::StandardItemData* Utils::GetSelectedItemDataInMenu() {
    if (const auto ui = RE::UI::GetSingleton()) {
        if (ui->IsMenuOpen(RE::InventoryMenu::MENU_NAME)) {
            return GetSelectedItemData<RE::InventoryMenu>();
        }
        if (ui->IsMenuOpen(RE::ContainerMenu::MENU_NAME)) {
            return GetSelectedItemData<RE::ContainerMenu>();
        }
        if (ui->IsMenuOpen(RE::BarterMenu::MENU_NAME)) {
            return GetSelectedItemData<RE::BarterMenu>();
        }
    }
    return nullptr;
}


RE::TESObjectREFR* Utils::GetOwnerOfItem(const RE::StandardItemData* a_itemdata) {
    auto& refHandle = a_itemdata->owner;
    if (const auto owner = RE::TESObjectREFR::LookupByHandle(refHandle)) {
        return owner.get();
    }
    if (const auto owner_actor = RE::Actor::LookupByHandle(refHandle)) {
        return owner_actor->AsReference();
    }
    return nullptr;
}