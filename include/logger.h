#pragma once

#include <cstdint>
#include <iostream>
#include <source_location>
#include <string_view>

#include "utils/bit_flags.h"

enum class LoggerCategory : uint8_t {
    None     = 0 << 0,
    Debug    = 1 << 0,
    Info     = 1 << 1,
    Warning  = 1 << 2,
    Error    = 1 << 3,
    Critical = 1 << 4,
};

using LoggerCategories = BitFlags<LoggerCategory>;

class Logger {
#ifdef NDEBUG
    static constexpr LoggerCategories DEFAULT_PRINT_FILTER{
        LoggerCategory::Critical, LoggerCategory::Error, LoggerCategory::Warning};
#else
    static constexpr LoggerCategories DEFAULT_PRINT_FILTER{
        LoggerCategory::Critical, LoggerCategory::Error, LoggerCategory::Warning,
        LoggerCategory::Info, LoggerCategory::Debug};
#endif

    // critical can technically be made no-throw, but there's a lot of code that expects a call to
    // Logger::critical_error to be terminating (or at least throwing), so only do that with care.
    // app behavior on the error path with a non-throwing critical setup is undefined
    static constexpr LoggerCategories DEFAULT_THROW_FILTER{LoggerCategory::Critical};

    static constexpr LoggerCategories DEFAULT_ERR_OUT_FILTER{
        LoggerCategory::Critical, LoggerCategory::Error, LoggerCategory::Warning};

    static constexpr LoggerCategories DEFAULT_LOC_FILTER{
        LoggerCategory::Critical, LoggerCategory::Error, LoggerCategory::Warning};

    using src_loc = std::source_location;

public:
    LoggerCategories print_filter   = DEFAULT_PRINT_FILTER;
    LoggerCategories throw_filter   = DEFAULT_THROW_FILTER;
    LoggerCategories err_out_filter = DEFAULT_ERR_OUT_FILTER;
    LoggerCategories loc_filter     = DEFAULT_LOC_FILTER;

    std::ostream& info_out = std::cout;
    std::ostream& err_out  = std::cerr;

    bool print_timestamps = false;

    static Logger& instance() {
        static Logger instance{};

        return instance;
    }

    void log(LoggerCategory category, std::string_view label, std::string_view message,
             bool msg_in_new_line = false, src_loc loc = src_loc::current());

    static void critical_error(std::string_view label, std::string_view message,
                               bool msg_in_new_line = false, src_loc loc = src_loc::current()) {
        return instance().log(LoggerCategory::Critical, label, message, msg_in_new_line, loc);
    }

    static void error(std::string_view label, std::string_view message,
                      bool msg_in_new_line = false, src_loc loc = src_loc::current()) {
        return instance().log(LoggerCategory::Error, label, message, msg_in_new_line, loc);
    }

    static void warn(std::string_view label, std::string_view message, bool msg_in_new_line = false,
                     src_loc loc = src_loc::current()) {
        return instance().log(LoggerCategory::Warning, label, message, msg_in_new_line, loc);
    }

    static void info(std::string_view label, std::string_view message, bool msg_in_new_line = false,
                     src_loc loc = src_loc::current()) {
        return instance().log(LoggerCategory::Info, label, message, msg_in_new_line, loc);
    }

    static void debug(std::string_view label, std::string_view message,
                      bool msg_in_new_line = false, src_loc loc = src_loc::current()) {
        return instance().log(LoggerCategory::Debug, label, message, msg_in_new_line, loc);
    }

    static void debug(std::string_view message, bool msg_in_new_line = false,
                      src_loc loc = src_loc::current()) {
        return Logger::debug("StdDbg", message, msg_in_new_line, loc);
    }

private:
    explicit Logger() {}
};
