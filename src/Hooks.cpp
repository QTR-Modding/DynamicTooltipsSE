#include "Hooks.h"
#include "Modules.h"

namespace {
    class MagicMenuSpellVisitor final : public RE::Actor::ForEachSpellVisitor {
    public:
        explicit MagicMenuSpellVisitor(Module& a_module) : module(a_module) {}

        RE::BSContainer::ForEachResult Visit(RE::SpellItem* a_spell) override {
            module.BuildSpellLoreCache(a_spell);
            return RE::BSContainer::ForEachResult::kContinue;
        }

    private:
        Module& module;
    };

    void BuildMagicLoreCache() {
        const auto module = Modules::modules.find(Modules::Modules::WhichMods);
        if (module == Modules::modules.end() || !module->second.IsEnabled()) {
            return;
        }

        if (const auto player = RE::PlayerCharacter::GetSingleton()) {
            MagicMenuSpellVisitor visitor(module->second);
            player->VisitSpells(visitor);
        }
    }

    void BuildCraftingLoreCache(RE::CraftingMenu* a_menu) {
        const auto module = Modules::modules.find(Modules::Modules::WhichMods);
        if (module == Modules::modules.end() || !module->second.IsEnabled()) {
            return;
        }

        RE::TESFurniture* furniture = nullptr;
        if (const auto player = RE::PlayerCharacter::GetSingleton()) {
            if (const auto furnitureRef = player->GetOccupiedFurniture().get();
                furnitureRef && furnitureRef->GetBaseObject()) {
                furniture = furnitureRef->GetBaseObject()->As<RE::TESFurniture>();
            }
        }
        if (!furniture && a_menu) {
            if (const auto subMenu = a_menu->GetCraftingSubMenu()) {
                furniture = subMenu->furniture;
            }
        }
        const auto dataHandler = RE::TESDataHandler::GetSingleton();
        if (!furniture ||
            furniture->workBenchData.benchType != RE::TESFurniture::WorkBenchData::BenchType::kCreateObject ||
            !dataHandler) {
            return;
        }

        for (const auto constructibleObject : dataHandler->GetFormArray<RE::BGSConstructibleObject>()) {
            if (constructibleObject && constructibleObject->CanBeCreatedOnWorkbench(furniture, false)) {
                module->second.BuildLoreCache(
                    constructibleObject->createdItem
                        ? constructibleObject->createdItem->As<RE::TESBoundObject>()
                        : nullptr);
            }
        }
    }
}

void Hooks::Install() {
    MenuHook<RE::ContainerMenu>::InstallHook();
    MenuHook<RE::InventoryMenu>::InstallHook();
    MenuHook<RE::BarterMenu>::InstallHook();
    MenuHook<RE::GiftMenu>::InstallHook();
    MenuHook<RE::MagicMenu>::InstallHook();
    MenuHook<RE::CraftingMenu>::InstallHook();
}

template <typename MenuType>
RE::UI_MESSAGE_RESULTS Hooks::MenuHook<MenuType>::ProcessMessage_Hook(RE::UIMessage& a_message) {
    const auto msg_type = static_cast<int>(a_message.type.get());
    if constexpr (std::is_same_v<MenuType, RE::MagicMenu>) {
        if (msg_type == 1) {
            BuildMagicLoreCache();
        } else if (msg_type == 3) {
            update_on_next = false;
            for (auto& a_sub : Modules::modules | std::views::values) {
                a_sub.ClearLoreCache();
            }
        }
        return _ProcessMessage(this, a_message);
    }

    if constexpr (std::is_same_v<MenuType, RE::CraftingMenu>) {
        if (msg_type == 1) {
            BuildCraftingLoreCache(this);
        } else if (msg_type == 3) {
            update_on_next = false;
            for (auto& a_sub : Modules::modules | std::views::values) {
                a_sub.ClearLoreCache();
            }
        }
        return _ProcessMessage(this, a_message);
    }

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
        if (MenuType::MENU_NAME == RE::GiftMenu::MENU_NAME) {
            if (RE::TESObjectREFRPtr gifter; LookupReferenceByHandle(RE::GiftMenu::GetGifterRefHandle(), gifter) &&
                                             gifter.get() != RE::PlayerCharacter::GetSingleton()) {
                const auto gifter_inv = gifter->GetInventory();
                for (auto& a_sub : Modules::modules | std::views::values) {
                    a_sub.BuildLoreCache(gifter_inv);
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
            RE::SendUIMessage::SendInventoryUpdateMessage(RE::PlayerCharacter::GetSingleton(), nullptr);
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
