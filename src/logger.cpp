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
        case LoggerCategory::Message:  return "MESSAGE";
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
                 bool msg_in_new_line) {
    using clock = std::chrono::system_clock;

    std::ostream& out = ERR_OUT_CATEGORIES.has(category) ? err_out : info_out;
    if (filter.has(category)) {
        if (print_timestamps)
            out << std::format("[{:%H:%M:%S}] ", clock::now());

        out << std::format("[{}] [{}]{}{}\n", log_cat_str(category), label,
                           msg_in_new_line ? '\n' : ' ', message);
    }

    if (throw_filter.has(category)) {
        log(LoggerCategory::Info, "Logger",
            "throw filter contains log category's bit, throwing runtime exception");

        throw std::runtime_error(std::string(message));
    }
}
