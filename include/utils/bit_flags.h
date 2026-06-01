#pragma once

#include <type_traits>

template <typename T>
requires std::is_enum_v<T>
struct BitFlags {
private:
    using IntT = std::underlying_type_t<T>;

    IntT bits{};

public:
    constexpr explicit BitFlags() = default;

    constexpr BitFlags(T initial_value)
        : bits{static_cast<IntT>(initial_value)} {}

    template <typename... Args>
    requires (std::same_as<BitFlags<T>, std::remove_cvref_t<Args>> && ...)
    constexpr explicit BitFlags(Args&&... args)
        : BitFlags{(args | ...)} {}

    template <typename... Args>
    requires (std::same_as<T, std::remove_cvref_t<Args>> && ...)
    constexpr explicit BitFlags(Args&&... args)
        : bits{(BitFlags<T>(args) | ...).get_bits()} {}

    constexpr BitFlags<T> operator|(BitFlags<T> other) const { return this->bits | other.bits; }
    constexpr BitFlags<T>& operator|=(BitFlags<T> other) {
        bits |= other.bits;
        return *this;
    }

    constexpr BitFlags<T> operator|(T flag) const { return this->bits | static_cast<IntT>(flag); }
    constexpr BitFlags<T>& operator|=(T flag) {
        bits |= static_cast<IntT>(flag);
        return *this;
    }

    constexpr BitFlags<T> operator&(BitFlags<T> other) const { return this->bits & other.bits; }
    constexpr BitFlags<T>& operator&=(BitFlags<T> other) {
        bits &= other.bits;
        return *this;
    }

    constexpr BitFlags<T> operator&(T flag) const { return this->bits & static_cast<IntT>(flag); }
    constexpr BitFlags<T>& operator&=(T flag) {
        bits &= static_cast<IntT>(flag);
        return *this;
    }

    constexpr IntT get_bits() const { return bits; }

    constexpr void enable(T target) { bits |= static_cast<IntT>(target); }

    constexpr void disable(T target) { bits &= ~static_cast<IntT>(target); }

    constexpr void toggle(T target) { bits ^= static_cast<IntT>(target); }

    constexpr bool has(T target) const { return bits & static_cast<IntT>(target); }

    template <typename... Args>
    requires (std::same_as<T, std::remove_cvref_t<Args>> && ...)
    constexpr bool has_any(Args&&... args) const {
        return (has(args) || ...);
    }

    template <typename... Args>
    requires (std::same_as<T, std::remove_cvref_t<Args>> && ...)
    constexpr bool has_all(Args&&... args) const {
        return (has(args) && ...);
    }

private:
    constexpr BitFlags(IntT initial_bits)
        : bits{initial_bits} {}
};
