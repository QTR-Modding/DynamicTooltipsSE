#include "Hooks.h"
#include "Settings.h"

void Hooks::Install() {
    MenuHook<RE::ContainerMenu>::InstallHook(RE::ContainerMenu::VTABLE[0]);
    MenuHook<RE::InventoryMenu>::InstallHook(RE::InventoryMenu::VTABLE[0]);
}

template <typename MenuType>
RE::UI_MESSAGE_RESULTS Hooks::MenuHook<MenuType>::ProcessMessage_Hook(RE::UIMessage& a_message) {
    const auto msg_type = static_cast<int>(a_message.type.get());
    if (msg_type != 3 && msg_type != 1) {
        return _ProcessMessage(this, a_message);
    }

    if (msg_type == 1) {
        const RE::TESObjectREFR::InventoryItemMap player_inv = RE::PlayerCharacter::GetSingleton()->GetInventory();
        for (auto& a_sub : Settings::subMods | std::views::values) {
            a_sub.BuildLoreCache(player_inv);
        }

        if (this->MENU_NAME == RE::ContainerMenu::MENU_NAME) {
            if (RE::TESObjectREFRPtr refr; LookupReferenceByHandle(RE::ContainerMenu::GetTargetRefHandle(), refr)) {
                const auto ref_inv = refr->GetInventory();
                for (auto& a_sub : Settings::subMods | std::views::values) {
                    a_sub.BuildLoreCache(ref_inv);
                }
            }
        }
    } else {
        for (auto& a_sub : Settings::subMods | std::views::values) {
            a_sub.ClearLoreCache();
        }
    }

    return _ProcessMessage(this, a_message);
}

template <typename MenuType>
void Hooks::MenuHook<MenuType>::InstallHook(const REL::VariantID& varID) {
    REL::Relocation vTable(varID);
    _ProcessMessage = vTable.write_vfunc(0x4, &MenuHook::ProcessMessage_Hook);
}