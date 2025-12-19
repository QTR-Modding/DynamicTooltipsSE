#include "DTF.h"
#include "Settings.h"
#include "Modules.h"
#include "Utils.h"

// ReSharper disable once CppParameterMayBeConst
const wchar_t* OnDynamicTranslationRequest(std::string_view a_key) {
    DTF::result_str.clear();

    for (auto& a_submod : Modules::modules | std::views::values) {
        if (a_key == a_submod.GetKeywordName()) {
            if (const auto lore = a_submod.GetLore(); !lore.empty()) {
                std::string title;
                if (Settings::show_titles) {
                    title = a_submod.GetTitle();
                }
                if (!title.empty()) {
                    // add color to the title
                    const auto packed = Utils::ConvertColor(a_submod.GetColor());
                    const uint32_t r = packed & 0xFF;
                    const uint32_t g = (packed >> 8) & 0xFF;
                    const uint32_t b = (packed >> 16) & 0xFF;

                    const uint32_t rgb = (r << 16) | (g << 8) | b; // 0xRRGGBB
                    const auto hex = fmt::format("{:06X}", rgb);
                    const auto titleHtml = fmt::format("<font color=\"#{}\">{}</font>", hex, title);
                    DTF::result_str = Utils::utf8_to_wstring(titleHtml);
                    DTF::result_str += L"\n";
                }
                DTF::result_str += Utils::utf8_to_wstring(lore);
            }
            break;
        }
    }
    return DTF::result_str.c_str();
}