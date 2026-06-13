#pragma once

template <typename... Ts>
inline constexpr bool dependent_false_v = false;

template <typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;
