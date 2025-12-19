#pragma once

namespace DTF {
    inline std::wstring result_str;
}

extern "C" __declspec(dllexport) const wchar_t* OnDynamicTranslationRequest(std::string_view a_key);