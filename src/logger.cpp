#include "logger.h"

#include <chrono>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

constexpr std::string_view log_cat_str(LoggerCategory cat) {
    // clang-format off
    switch (cat) {
        case LoggerCategory::Debug:    return "DEBUG";
        case LoggerCategory::Info:     return "INFO";
        case LoggerCategory::Warning:  return "WARNING";
        case LoggerCategory::Error:    return "ERROR";
        case LoggerCategory::Critical: return "CRITICAL";
        default:                       return "UNKNOWN LOGGER CATEGORY";
    }
    // clang-format on
}

} // namespace

void Logger::log(LoggerCategory category, std::string_view label, std::string_view message,
                 bool msg_in_new_line, src_loc loc) {
    using clock = std::chrono::system_clock;

    std::ostream& out = err_out_filter.has(category) ? err_out : info_out;
    if (print_filter.has(category)) {
        if (print_timestamps)
            out << std::format("[{:%H:%M:%S}] ", clock::now()); // TODO

        out << std::format("[{}] [{}]{}{}\n", log_cat_str(category), label,
                           msg_in_new_line ? '\n' : ' ', message);

        if (loc_filter.has(category))
            out << std::format(" [{}:{}:{}]", loc.file_name(), loc.line(), loc.column());
    }

    static bool in_throw_state = false;
    if (!in_throw_state && throw_filter.has(category)) {
        in_throw_state = true;

        log(LoggerCategory::Info, "Logger",
            "throw filter contains log category's bit, throwing runtime exception");

        // reset to keep things working next throw if the current exception is caught
        in_throw_state = false;

        throw std::runtime_error(std::string(message));
    }
}
