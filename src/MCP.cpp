#include "MCP.h"
#include "Settings.h"
#include "Modules.h"
#include "SKSEMCP/SKSEMenuFramework.hpp"
#include "imgui.h"

void MCP::RenderSettings() {
    bool changed = false;
    for (auto& [moduleId, subMod] : Modules::modules) {
        const auto moduleName = Modules::ToString(moduleId);
        auto a_title = subMod.GetTitle();
        if (ImGui::CollapsingHeader(a_title.data())) {
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
    if (ImGuiMCP::Checkbox("Show Titles in Tooltips", &Settings::show_titles)) {
        changed = true;
    }
    if (ImGuiMCP::Checkbox("Disallow Editor IDs in Tooltips", &Settings::disallow_editorIDs)) {
        changed = true;
    }
    if (ImGuiMCP::Checkbox("Show Spell Levels in Tooltips", &Settings::show_spell_levels)) {
        changed = true;
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
