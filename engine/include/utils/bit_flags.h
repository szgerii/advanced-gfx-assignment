#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

template <typename E>
requires std::is_enum_v<E> && requires {
    { E::None } -> std::same_as<E>;
}
struct BitFlags {
private:
    using U = std::underlying_type_t<E>;

    U bits_{};

public:
    constexpr explicit BitFlags()
        : BitFlags{E::None} {}

    constexpr BitFlags(E initial_value)
        : bits_{static_cast<U>(initial_value)} {}

    template <typename... BFArgs>
    requires (sizeof...(BFArgs) > 1) &&
             (std::same_as<BitFlags<E>, std::remove_cvref_t<BFArgs>> && ...)
    constexpr BitFlags(BFArgs&&... args)
        : BitFlags{(std::forward<BFArgs>(args) | ...)} {}

    template <typename... EArgs>
    requires (sizeof...(EArgs) > 0) && (std::same_as<E, std::remove_cvref_t<EArgs>> && ...)
    constexpr BitFlags(EArgs&&... args)
        : bits_{(BitFlags(std::forward<EArgs>(args)) | ...).get_bits()} {}

    constexpr BitFlags<E> operator|(BitFlags<E> other) const { return this->bits_ | other.bits_; }
    constexpr BitFlags<E>& operator|=(BitFlags<E> other) {
        bits_ |= other.bits_;
        return *this;
    }

    constexpr BitFlags<E> operator|(E flag) const { return this->bits_ | static_cast<U>(flag); }
    constexpr BitFlags<E>& operator|=(E flag) {
        bits_ |= static_cast<U>(flag);
        return *this;
    }

    constexpr BitFlags<E> operator&(BitFlags<E> other) const { return this->bits_ & other.bits_; }
    constexpr BitFlags<E>& operator&=(BitFlags<E> other) {
        bits_ &= other.bits_;
        return *this;
    }

    constexpr BitFlags<E> operator&(E flag) const { return this->bits_ & static_cast<U>(flag); }
    constexpr BitFlags<E>& operator&=(E flag) {
        bits_ &= static_cast<U>(flag);
        return *this;
    }

    constexpr U get_bits() const { return bits_; }

    constexpr bool empty() const { return bits_ == static_cast<U>(E::None); }

    constexpr void enable(E target) { bits_ |= static_cast<U>(target); }

    constexpr void disable(E target) { bits_ &= ~static_cast<U>(target); }

    constexpr void toggle(E target) { bits_ ^= static_cast<U>(target); }

    constexpr bool has(E target) const { return bits_ & static_cast<U>(target); }

    template <typename... EArgs>
    requires (sizeof...(EArgs) > 0) && (std::same_as<E, std::remove_cvref_t<EArgs>> && ...)
    constexpr bool has_any(EArgs&&... args) const {
        return (has(std::forward<EArgs>(args)) || ...);
    }

    template <typename... EArgs>
    requires (sizeof...(EArgs) > 0) && (std::same_as<E, std::remove_cvref_t<EArgs>> && ...)
    constexpr bool has_all(EArgs&&... args) const {
        return (has(std::forward<EArgs>(args)) && ...);
    }

private:
    constexpr BitFlags(U initial_bits)
        : bits_{initial_bits} {}
};
