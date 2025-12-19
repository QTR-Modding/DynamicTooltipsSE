#include "Hooks.h"
#include "MCP.h"
#include "Settings.h"
#include "logger.h"

namespace {
    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    void OnMessage(SKSE::MessagingInterface::Message* message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            SKSE::Translation::ParseTranslation("DynamicTooltips");
            Settings::Load();
            MCP::Register();
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SetupLog();
    logger::info("Plugin loaded");
    SKSE::Init(skse);
    Hooks::Install();
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    return true;
}