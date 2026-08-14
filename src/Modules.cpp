#include "Modules.h"
#include "Utils.h"


std::string Module::GetLore() const {
    if (const auto a_entry = Utils::GetSelectedEntryInMenu()) {
        return features.getLore(a_entry);
    }
    if (const auto a_obj = Utils::GetSelectedCraftingObject()) {
        RE::InventoryEntryData craftingEntry(a_obj, 1);
        return features.getLore(&craftingEntry);
    }
    if (const auto a_obj = Utils::GetSelectedMagicObject()) {
        RE::InventoryEntryData magicEntry(a_obj, 1);
        return features.getLore(&magicEntry);
    }
    return "";
}

void Module::BuildLoreCache(RE::TESBoundObject* a_obj) {
    if (!a_obj || a_obj->Is(RE::FormType::LeveledItem) ||
        (!a_obj->Is(RE::FormType::Spell) && !a_obj->GetPlayable())) {
        return;
    }

    if (a_obj->Is(RE::FormType::Spell)) {
        if (const auto spell = a_obj->As<RE::SpellItem>()) {
            for (const auto effect : spell->effects) {
                if (effect && effect->baseEffect && !loreCache.contains(effect->baseEffect)) {
                    effect->baseEffect->AddKeyword(kw);
                    loreCache.insert(effect->baseEffect);
                }
            }
        }
        return;
    }

    RE::BGSKeywordForm* a_kw_form;
    if (const auto ammo = a_obj->As<RE::TESAmmo>()) {
        a_kw_form = ammo->AsKeywordForm();
    } else {
        a_kw_form = a_obj->As<RE::BGSKeywordForm>();
    }
    if (!a_kw_form || loreCache.contains(a_obj)) {
        return;
    }

    a_kw_form->AddKeyword(kw);
    loreCache.insert(a_obj);
}

void Module::BuildLoreCache(const RE::TESObjectREFR::InventoryItemMap& a_inv) {
    for (const auto& [obj, entry] : a_inv) {
        if (entry.first > 0) {
            BuildLoreCache(obj);
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
        BuildLoreCache(obj);
    }
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
