#pragma once

#include <cmath>
#include <concepts>
#include <format>
#include <limits>
#include <optional>
#include <utility>

#include "core/logger.h"
#include "utils/common.h"

// util for wrapping a numeric static_cast in a runtime debug check that it's indeed well-defined
// and lossless. use with values that should have the same value once cast to To
// (e.g.: converting a >= 0 signed value to an unsigned type)
// do NOT use when well-defined truncation or rounding is acceptible
template <typename To, typename From>
requires (std::integral<To> || std::floating_point<To>) &&
         (std::integral<From> || std::floating_point<From>)
constexpr To num_cast(From value)
#ifdef NDEBUG // Logger::warn may throw if throw_filter has the warning bit set
    noexcept
#endif
{
#ifndef NDEBUG
    auto check_spec_val = [](auto val) -> std::optional<std::string_view> {
        if (std::isinf(val))
            return !std::signbit(val) ? "+inf" : "-inf";
        else if (std::isnan(val))
            return "NaN";

        return std::nullopt;
    };

    // in debug builds, logger warns when used in a runtime setting and stops compilation when
    // executed in a constexpr context
    // this lambda passing is kinda ugly, but needed to allow const execution without indirectly
    // masking the actual source of the error behind invalid std::format constant-time calls
    auto report_error = [](auto get_runtime_msg) {
        if (std::is_constant_evaluated())
            throw "invalid numeric cast used in constant evaluated context";
        else
            Logger::warn("num_cast", get_runtime_msg());
    };

    if constexpr (std::is_same_v<To, From>) {
        report_error([=]() { return "conversion between identical numeric types"; });
    } else if constexpr (std::integral<From> && std::integral<To>) {
        if (!std::in_range<To>(value)) {
            report_error([=]() {
                return std::format("narrowing conversion occurred when converting "
                                   "value between integral types ({} -> {})",
                                   value, static_cast<To>(value));
            });
        }
    } else if constexpr (std::floating_point<From> && std::floating_point<To>) {
        auto spec_val = check_spec_val(value);
        if (spec_val.has_value()) {
            report_error([=]() {
                return std::format(
                    "conversion of non-numeric value '{}' between floating point types", *spec_val);
            });
        } else if (static_cast<From>(static_cast<To>(value)) != value) {
            report_error([=]() {
                return std::format("narrowing conversion occured while converting "
                                   "value between floating point types ({} -> {})",
                                   value, static_cast<To>(value));
            });
        }
    } else if constexpr (std::integral<From> && std::floating_point<To>) {
        // the extra bounds check is required because e.g. UINT64_MAX to float
        // would yield 2^64 as a float which cannot then be cast back to uint64_t

        To casted = static_cast<To>(value);
        To excl_upper =
            static_cast<To>(static_cast<To>(std::numeric_limits<From>::max()) / 2.0 + 1) * 2.0;

        if (casted >= excl_upper || static_cast<From>(casted) != value) {
            report_error([=]() {
                return std::format("narrowing conversion occured while converting "
                                   "value from integral to floating point type ({} -> {})",
                                   value, static_cast<To>(value));
            });
        }
    } else {
        // fp -> int
        auto spec_val = check_spec_val(value);
        if (spec_val.has_value()) {
            report_error([=]() {
                return std::format("conversion of non-numeric value '{}' from "
                                   "floating point to integral type (undefined behavior)",
                                   *spec_val);
            });
        } else if (static_cast<From>(static_cast<To>(value)) != value) {
            report_error([=]() {
                return std::format(
                    "fractional truncation or narrowing conversion occured while "
                    "converting value from floating point to integral type ({} -> {})",
                    value, static_cast<To>(value));
            });
        }
    }
#endif

    return static_cast<To>(value);
}
