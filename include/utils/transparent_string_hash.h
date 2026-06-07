#pragma once

#include <string>
#include <string_view>

// for heterogenous lookup in unordered STL containers
// source: https://www.cppstories.com/2021/heterogeneous-access-cpp20/
struct TransparentStringHash {
    using is_transparent = void;

    [[nodiscard]] size_t operator()(const char* c_str) const {
        return std::hash<std::string_view>{}(c_str);
    }

    [[nodiscard]] size_t operator()(std::string_view sv) const {
        return std::hash<std::string_view>{}(sv);
    }

    [[nodiscard]] size_t operator()(const std::string& str) const {
        return std::hash<std::string>{}(str);
    }
};
