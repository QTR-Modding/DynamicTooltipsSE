#include "MCP.h"
#include "Settings.h"
#include "Modules.h"
#include "SKSEMCP/SKSEMenuFramework.hpp"

namespace {
    const char* GetModuleDescription(Modules::Modules a_id) {
        switch (a_id) {
            case Modules::Modules::WhoseQuest:
                return "Shows which quest a quest item belongs to.";
            case Modules::Modules::WhoseItem:
                return "Shows who owns an item.";
            case Modules::Modules::SPBMGCK:
                return "Shows spell tome info such as spell level and magicka cost.\n"
                    "If the cost exceeds your max magicka, the number is tinted pink-ish.";
            default:
                return "";
        }
    }

    void DrawCategoryDescription(const char* a_text) {
        if (a_text && a_text[0] != '\0') {
            ImGuiMCP::TextWrapped("%s", a_text);
            ImGuiMCP::Spacing();
            ImGuiMCP::Separator();
            ImGuiMCP::Spacing();
        }
    }
}

void MCP::RenderSettings() {
    bool changed = false;

    // --- Modules ---
    for (auto& [moduleId, subMod] : Modules::modules) {
        const auto moduleName = Modules::ToString(moduleId);
        const auto title = subMod.GetTitle();

        if (ImGuiMCP::CollapsingHeader(title.data())) {
            DrawCategoryDescription(GetModuleDescription(moduleId));

            bool enabled = static_cast<bool>(subMod);
            const auto enabledLabel = std::string("Enabled##") + moduleName;
            if (ImGuiMCP::Checkbox(enabledLabel.c_str(), &enabled)) {
                subMod.Toggle(enabled);
                changed = true;
            }

            const auto a_color = subMod.GetColor();
            float col[3] = {a_color.red, a_color.green, a_color.blue};
            const auto colorLabel = std::string("Title Color##") + moduleName;
            if (ImGuiMCP::ColorEdit3(colorLabel.c_str(), col)) {
                subMod.ChangeColor(RE::NiColor(col[0], col[1], col[2]));
                changed = true;
            }
        }
    }

    // --- Other / Global settings ---
    if (ImGuiMCP::CollapsingHeader("Other")) {
        DrawCategoryDescription("General settings that affect how all tooltip modules are rendered.");

        if (ImGuiMCP::Checkbox("Show Titles in Tooltips", &Settings::show_titles)) {
            changed = true;
        }
        ImGuiMCP::Text("Adds the module title line above the tooltip text.");

        if (ImGuiMCP::Checkbox("Disallow Editor IDs in Tooltips", &Settings::disallow_editorIDs)) {
            changed = true;
        }
        ImGuiMCP::Text("Prevents fallback to editor IDs when a display name is missing.");

        if (ImGuiMCP::Checkbox("Show Spell Levels in Tooltips", &Settings::show_spell_levels)) {
            changed = true;
        }
        ImGuiMCP::Text("Adds Novice/Apprentice/Adept/Expert/Master when available.");
    }

    if (changed) {
        Settings::Save();
    }
}


void MCP::Register() {
    if (!SKSEMenuFramework::IsInstalled()) {
        return;
    }
    SKSEMenuFramework::SetSection(Settings::mod_name);
    SKSEMenuFramework::AddSectionItem("Settings", RenderSettings);
    logger::info("MCP registered.");
}