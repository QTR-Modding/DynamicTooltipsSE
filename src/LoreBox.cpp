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
    if (const auto a_book = a_obj->As<RE::TESObjectBOOK>()) {
        if (const auto a_spell = a_book->GetSpell()) {
            float a_cost = a_spell->CalculateMagickaCost(RE::PlayerCharacter::GetSingleton());
            return fmt::format("{:.0f} Magicka", a_cost);
        }
    }
    return "";
}