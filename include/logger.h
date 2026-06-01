#pragma once

#include <cstdint>
#include <iostream>
#include <string_view>

#include "utils/bit_flags.h"

enum class LoggerCategory : uint8_t {
    None     = 0,
    Message  = 1 << 0,
    Info     = 1 << 1,
    Warning  = 1 << 2,
    Error    = 1 << 3,
    Critical = 1 << 4,
};

using LoggerCategories = BitFlags<LoggerCategory>;

class Logger {
#ifdef NDEBUG
    static constexpr LoggerCategories DEFAULT_FILTER{
        LoggerCategory::Critical, LoggerCategory::Error, LoggerCategory::Warning};
#else
    static constexpr LoggerCategories DEFAULT_FILTER{LoggerCategory::Critical,
                                                     LoggerCategory::Error, LoggerCategory::Warning,
                                                     LoggerCategory::Info, LoggerCategory::Message};
#endif

    static constexpr LoggerCategories ERR_OUT_CATEGORIES{
        LoggerCategory::Critical, LoggerCategory::Error, LoggerCategory::Warning};

public:
    LoggerCategories filter       = DEFAULT_FILTER;
    LoggerCategories throw_filter = LoggerCategory::Critical;

    std::ostream& info_out = std::cout;
    std::ostream& err_out  = std::cerr;

    bool print_timestamps = false;

    static Logger& instance() {
        static Logger instance{};

        return instance;
    }

    void log(LoggerCategory category, std::string_view label, std::string_view message,
             bool msg_in_new_line = false);

    template <typename... Args>
    void critical_error(Args&&... args) {
        return log(LoggerCategory::Critical, args...);
    }

    template <typename... Args>
    void error(Args&&... args) {
        return log(LoggerCategory::Error, args...);
    }

    template <typename... Args>
    void warn(Args&&... args) {
        return log(LoggerCategory::Warning, args...);
    }

    template <typename... Args>
    void info(Args&&... args) {
        return log(LoggerCategory::Info, args...);
    }

    template <typename... Args>
    void message(Args&&... args) {
        return log(LoggerCategory::Message, args...);
    }

private:
    explicit Logger() {}
};
