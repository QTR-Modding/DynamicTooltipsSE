#include "Modules.h"
#include "Utils.h"


std::string Module::GetLore() const {
    if (const auto a_entry = Utils::GetSelectedEntryInMenu()) {
        return features.getLore(a_entry);
    }
    return "";
}

void Module::BuildLoreCache(const RE::TESObjectREFR::InventoryItemMap& a_inv) {
    for (const auto& [obj, entry] : a_inv) {
        if (entry.first <= 0 || !obj->GetPlayable() || obj->Is(RE::FormType::LeveledItem)) {
            continue;
        }
        RE::BGSKeywordForm* a_kw_form;
        if (const auto ammo = obj->As<RE::TESAmmo>()) {
            a_kw_form = ammo->AsKeywordForm();
        } else {
            a_kw_form = obj->As<RE::BGSKeywordForm>();
        }
        if (!a_kw_form) {
            continue;
        }

        if (!loreCache.contains(obj)) {
            a_kw_form->AddKeyword(kw);
            loreCache.insert(obj);
        }
    }
}

void Module::BuildLoreCache(const RE::ItemList* a_itemList) {
    if (!a_itemList) {
        logger::error("BuildLoreCache called with null ItemList");
        return;
    }
    for (const auto& item : a_itemList->items) {
        #undef GetObject
        const auto obj = item->data.objDesc->GetObject();
        if (!obj || !obj->GetPlayable() || obj->Is(RE::FormType::LeveledItem)) {
            continue;
        }
        RE::BGSKeywordForm* a_kw_form;
        if (const auto ammo = obj->As<RE::TESAmmo>()) {
            a_kw_form = ammo->AsKeywordForm();
        } else {
            a_kw_form = obj->As<RE::BGSKeywordForm>();
        }
        if (!a_kw_form) {
            continue;
        }
        if (!loreCache.contains(obj)) {
            a_kw_form->AddKeyword(kw);
            loreCache.insert(obj);
        }
    }
    RE::SendUIMessage::SendInventoryUpdateMessage(RE::PlayerCharacter::GetSingleton(), nullptr);
}

void Module::ClearLoreCache() {
    for (const auto a_obj : loreCache) {
        RE::BGSKeywordForm* a_kw_form;
        if (const auto ammo = a_obj->As<RE::TESAmmo>()) {
            a_kw_form = ammo->AsKeywordForm();
        } else {
            a_kw_form = a_obj->As<RE::BGSKeywordForm>();
        }
        if (a_kw_form) {
            a_kw_form->RemoveKeyword(kw);
        }
    }
    loreCache.clear();
}

Module::Module(const std::string& a_name, const ModuleFeatures& a_features) : features(a_features) {
    if (!SKSE::Translation::Translate("$" + a_name + "Title", title)) {
        logger::error("Failed to translate title for sub-mod '{}'", a_name);
    }
    if (kw = Utils::MakeKeyword("LoreBox_" + a_name); !kw) {
        logger::error("Failed to find keyword for sub-mod '{}'", a_name);
    }
}

void Module::Toggle(const bool a_enable) {
    features.enabled = a_enable;
    if (!features.enabled) {
        ClearLoreCache();
    }
}

RE::NiColor Module::GetColor() const { return features.titleColor; }

void Module::ChangeColor(const RE::NiColor& a_color) { features.titleColor = a_color; }

std::string Module::GetKeywordName() const { return kw->GetFormEditorID(); }