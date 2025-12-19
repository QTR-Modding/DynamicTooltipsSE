#include "Hooks.h"
#include "Modules.h"

void Hooks::Install() {
    MenuHook<RE::ContainerMenu>::InstallHook();
    MenuHook<RE::InventoryMenu>::InstallHook();
    MenuHook<RE::BarterMenu>::InstallHook();
}

template <typename MenuType>
RE::UI_MESSAGE_RESULTS Hooks::MenuHook<MenuType>::ProcessMessage_Hook(RE::UIMessage& a_message) {
    const auto msg_type = static_cast<int>(a_message.type.get());
    if (msg_type == 1) {
        const RE::TESObjectREFR::InventoryItemMap player_inv = RE::PlayerCharacter::GetSingleton()->GetInventory();
        for (auto& a_sub : Modules::modules | std::views::values) {
            a_sub.BuildLoreCache(player_inv);
        }

        if (MenuType::MENU_NAME == RE::ContainerMenu::MENU_NAME) {
            if (RE::TESObjectREFRPtr refr; LookupReferenceByHandle(RE::ContainerMenu::GetTargetRefHandle(), refr)) {
                const auto ref_inv = refr->GetInventory();
                for (auto& a_sub : Modules::modules | std::views::values) {
                    a_sub.BuildLoreCache(ref_inv);
                }
            }
        }
        if (MenuType::MENU_NAME == RE::BarterMenu::MENU_NAME) {
            update_on_next = true;
        }
    } else if (msg_type == 0 && update_on_next) {
        update_on_next = false;
        if (MenuType::MENU_NAME == RE::BarterMenu::MENU_NAME) {
            const auto bartermenu = RE::UI::GetSingleton()->GetMenu<RE::BarterMenu>();
            for (auto& a_sub : Modules::modules | std::views::values) {
                a_sub.BuildLoreCache(bartermenu->GetRuntimeData().itemList);
            }
        }
    } else if (msg_type == 3) {
        update_on_next = false;
        for (auto& a_sub : Modules::modules | std::views::values) {
            a_sub.ClearLoreCache();
        }
    }

    return _ProcessMessage(this, a_message);
}

template <typename MenuType>
void Hooks::MenuHook<MenuType>::InstallHook() {
    REL::Relocation vTable(MenuType::VTABLE[0]);
    _ProcessMessage = vTable.write_vfunc(0x4, &MenuHook::ProcessMessage_Hook);
}