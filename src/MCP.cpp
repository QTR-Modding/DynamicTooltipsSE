#include "MCP.h"
#include "Settings.h"
#include "SKSEMCP/SKSEMenuFramework.hpp"

void MCP::RenderSettings() {
    bool changed = false;
    for (auto& subMod : Settings::subMods | std::views::values) {
        bool enabled = static_cast<bool>(subMod);
        auto a_title = subMod.GetTitle();
        if (ImGuiMCP::Checkbox(a_title.data(), &enabled)) {
            subMod.Toggle(enabled);
            changed = true;
        }
        const auto a_color = subMod.GetColor();
        float col[3] = {a_color.red, a_color.green, a_color.blue};
        if (ImGuiMCP::ColorEdit3((std::string(a_title) + "_color").c_str(), col)) {
            subMod.ChangeColor(RE::NiColor(col[0], col[1], col[2]));
            changed = true;
        }
    }
    if (ImGuiMCP::Checkbox("Disallow Editor IDs in Tooltips", &Settings::disallow_editorIDs)) {
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