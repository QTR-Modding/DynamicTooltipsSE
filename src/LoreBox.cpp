#include "LoreBox.h"
#include "Settings.h"
#include "Utils.h"
#include "ClibUtil/editorID.hpp"

std::string LoreBox::GetLoreWQ(const RE::InventoryEntryData* a_entryData) {
    if (const auto a_quest = Utils::GetQuest(a_entryData)) {
        std::string a_name;
        if (a_name = a_quest->GetFullName(); !a_name.empty()) {
            return a_name;
        }
        if (a_name = a_quest->GetName(); !a_name.empty()) {
            return a_name;
        }
        if (!Settings::disallow_editorIDs) {
            if (a_name = clib_util::editorID::get_editorID(a_quest); !a_name.empty()) {
                return a_name;
            }
        }
    }
    return "";
}

std::string LoreBox::GetLoreIO(const RE::InventoryEntryData* a_entryData) {
    if (const auto a_owner = Utils::GetOwner(a_entryData); a_owner && !a_owner->IsPlayer() && !a_owner->IsPlayerRef()) {
        if (const auto a_fullnameform = a_owner->As<RE::TESFullName>()) {
            if (const auto a_name = a_fullnameform->GetFullName(); !Utils::is_empty(a_name)) {
                return a_name;
            }
        }
        if (const auto a_name = a_owner->GetName(); !Utils::is_empty(a_name)) {
            return a_name;
        }
        if (!Settings::disallow_editorIDs) {
            if (const auto a_name = clib_util::editorID::get_editorID(a_owner); !a_name.empty()) {
                return a_name;
            }
        }
    }
    return "";
}

std::string LoreBox::GetLoreSPBMGCK(RE::InventoryEntryData* a_entryData) {
    #undef GetObject
    const auto a_obj = a_entryData->GetObject();
    const auto a_book = a_obj->As<RE::TESObjectBOOK>();
    if (!a_book) {
        return "";
    }
    const auto a_spell = a_book->GetSpell();
    if (!a_spell) {
        return "";
    }
    const auto player = RE::PlayerCharacter::GetSingleton();

    const float a_cost = a_spell->CalculateMagickaCost(player);
    auto costText = fmt::format("{:.0f} Magicka", a_cost);
    const float maxMagicka = player->GetActorValueMax(RE::ActorValue::kMagicka);
    if (a_cost > maxMagicka) {
        costText = fmt::format("<font color=\"{}\">{}</font>", Utils::kUnaffordableMagickaHtmlColor, costText);
    }

    std::string levelText;
    if (Settings::show_spell_levels) {
        switch (a_spell->GetSpellLevel()) {
            case RE::MagicSystem::SpellLevel::kNovice:
                levelText = "Novice";
                break;
            case RE::MagicSystem::SpellLevel::kApprentice:
                levelText = "Apprentice";
                break;
            case RE::MagicSystem::SpellLevel::kAdept:
                levelText = "Adept";
                break;
            case RE::MagicSystem::SpellLevel::kExpert:
                levelText = "Expert";
                break;
            case RE::MagicSystem::SpellLevel::kMaster:
                levelText = "Master";
                break;
            default:
                break;
        }
    }

    if (!levelText.empty()) {
        return fmt::format("{} | {}", levelText, costText);
    }
    return costText;
}
