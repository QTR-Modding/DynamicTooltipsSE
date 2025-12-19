#include "MCP.h"
#include "Settings.h"
#include "SKSEMCP/SKSEMenuFramework.hpp"

void MCP::RenderSettings() {
    for (auto& subMod : Settings::subMods | std::views::values) {
        bool enabled = static_cast<bool>(subMod);
        auto a_title = subMod.GetTitle();
        if (ImGuiMCP::Checkbox(a_title.data(), &enabled)) {
            subMod.Toggle(enabled);
        }
        ImGuiMCP::SameLine();
        RE::NiColorA a_color = subMod.GetColor();
        if (ImGuiMCP::ColorEdit4((std::string(a_title) + "_color").c_str(), reinterpret_cast<float*>(&a_color), 0)) {
            subMod.ChangeColor(a_color);
        }
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