#include "LoreBox.h"
#include "Settings.h"
#include "Utils.h"
#include "ClibUtil/editorID.hpp"

namespace {
    std::string GetSpellLevelFromPerk(const RE::SpellItem* a_spell) {
        if (!a_spell) {
            return "";
        }
        const auto castingPerk = a_spell->data.castingPerk;
        if (!castingPerk) {
            return "";
        }
        const auto perkId = castingPerk->GetFormID();
        switch (perkId) {
            // Novice perks
            case 0x000F2CA1: // Novice Alteration
            case 0x000F2CA2: // Novice Conjuration
            case 0x000F2CA3: // Novice Illusion
            case 0x000F2CA5: // Novice Restoration
            case 0x000F2CA6: // Novice Destruction
                return "Novice";
            // Apprentice perks
            case 0x000C44B7: // Apprentice Alteration
            case 0x000C44BB: // Apprentice Conjuration
            case 0x000C44C3: // Apprentice Illusion
            case 0x000C44C7: // Apprentice Restoration
            case 0x000C44BF: // Apprentice Destruction
                return "Apprentice";
            // Adept perks
            case 0x000C44B8: // Adept Alteration
            case 0x000C44BC: // Adept Conjuration
            case 0x000C44C4: // Adept Illusion
            case 0x000C44C8: // Adept Restoration
            case 0x000C44C0: // Adept Destruction
                return "Adept";
            // Expert perks
            case 0x000C44B9: // Expert Alteration
            case 0x000C44BD: // Expert Conjuration
            case 0x000C44C5: // Expert Illusion
            case 0x000C44C9: // Expert Restoration
            case 0x000C44C1: // Expert Destruction
                return "Expert";
            // Master perks
            case 0x000C44BA: // Master Alteration
            case 0x000C44BE: // Master Conjuration
            case 0x000C44C6: // Master Illusion
            case 0x000C44CA: // Master Restoration
            case 0x000C44C2: // Master Destruction
                return "Master";
            default:
                return "";
        }
    }
}

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
        levelText = GetSpellLevelFromPerk(a_spell);
    }

    if (!levelText.empty()) {
        return fmt::format("{} | {}", levelText, costText);
    }
    return costText;
}
