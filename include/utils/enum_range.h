#pragma once

#include <concepts>
#include <limits>
#include <ranges>
#include <type_traits>

#include "utils/common.h"

template <typename E>
concept CRangedEnum = std::is_enum_v<E> && requires {
    { E::FIRST } -> std::same_as<E>;
    { E::LAST } -> std::same_as<E>;
};

// handles enum value distances in an overflow-safe manner
template <typename E>
requires std::is_enum_v<E>
constexpr size_t enum_count(E start, E end) {
    using U    = std::underlying_type_t<E>;
    using Diff = std::make_unsigned_t<U>;

    // move to unsigned type to ensure no overflow during difference calculation, then widen to
    // size_t to ensure +1 doesnt overflow either
    // two's complement allows this regardless of signedness [https://stackoverflow.com/a/52582210]
    Diff distance = static_cast<Diff>(end) - static_cast<Diff>(start);
    size_t count  = start <= end ? static_cast<size_t>(distance) + 1 : 0ULL;

    return count;
}

template <typename E>
requires CRangedEnum<E>
constexpr size_t enum_count() {
    return enum_count(E::FIRST, E::LAST);
}

// range for continuos enum subranges
template <typename E>
requires std::is_enum_v<E>
constexpr auto enum_range(E range_start, E range_end) {
    using U = std::underlying_type_t<E>;

    size_t count = enum_count(range_start, range_end);

    // we could also use unbounded iota, but then range_end needs to be widened, which would lead to
    // more signed/unsigned and width expansion logic so ehh

    return (std::views::iota(static_cast<U>(range_start))) | std::views::take(count) |
           std::views::transform([](U it) { return static_cast<E>(it); });
}

// range for FIRST/LAST-marked, continous enums
template <typename E>
requires CRangedEnum<E>
constexpr auto enum_range() {
    using U = std::underlying_type_t<E>;

    if constexpr (static_cast<U>(E::FIRST) <= static_cast<U>(E::LAST)) {
        return enum_range(E::FIRST, E::LAST);
    } else {
        static_assert(dependent_false_v<E>,
                      "enum_range called with invalid enum type (LAST's value precedes FIRST's)");
    }
}
