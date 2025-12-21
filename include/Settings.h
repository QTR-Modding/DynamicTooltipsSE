#pragma once

namespace Settings {
    inline std::string mod_name = "Dynamic Tooltips";
    inline bool disallow_editorIDs = false;
    inline bool show_titles = true;
    inline bool show_spell_levels = true;
    inline int max_mod_names = 6;

    void Load();
    void Save();

    namespace INI {
        inline std::string path = "Data\\SKSE\\Plugins\\DynamicTooltips\\DynamicTooltips.ini";
    }
}