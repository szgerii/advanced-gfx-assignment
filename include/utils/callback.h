#pragma once

#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <utility>

// general, simple callback interface wrapping regular fn pointers with member fn pointers
// it's simple, crude, hacky, ugly, but it works and should be faster than std::function
// since callbacks are used in hot paths, this is just a first step for reducing callback overhead

// TODO
// this can be replaced with faster, not __thiscall-bound delegates
// example reference (only available via web archive):
// https://web.archive.org/web/20240510193429/https://www.codeproject.com/Articles/11015/The-Impossibly-Fast-C-Delegates

// do note that forwarding happens based on type-level signature, not on the actual call site's sig
// that means if you want an rvalue reference, it has to be explicitly set as T&& in Args
// (this means that it should be able to skip multiple moves, i think?)

template <typename Ret = void, typename... Args>
struct Callback {
    const void* object                  = nullptr;
    Ret (*invoke)(const void*, Args...) = nullptr;

    bool bound() const { return invoke != nullptr; }

    // perfect forwarding should save a move here for rvalue references (i think?)
    template <typename... CallArgs>
    requires std::is_invocable_r_v<Ret, decltype(invoke), const void*, CallArgs...>
    Ret operator()(CallArgs&&... args) const {
        if (invoke == nullptr)
            std::logic_error{"Callback with no inner function set was called"};

        return invoke(object, std::forward<CallArgs>(args)...);
    }

    // it is the caller's responsibility that the created callback does not outlive 'object'
    template <auto MemberFn, typename T>
    requires std::is_invocable_r_v<Ret, decltype(MemberFn), T*, Args...>
    static Callback<Ret, Args...> from(T* object) {
        assert(object != nullptr);
        return {object, [](const void* obj, Args... args) -> Ret {
                    // forgive me, for i have const_cast
                    return (static_cast<T*>(const_cast<void*>(obj))->*MemberFn)(
                        std::forward<Args>(args)...);
                }};
    }

    // needed to support const member functions
    template <auto MemberFn, typename T>
    requires std::is_invocable_r_v<Ret, decltype(MemberFn), const T*, Args...>
    static Callback<Ret, Args...> from(const T* object) {
        assert(object != nullptr);
        return {object, [](const void* obj, Args... args) -> Ret {
                    return (static_cast<const T*>(obj)->*MemberFn)(std::forward<Args>(args)...);
                }};
    }

    template <auto Function>
    requires std::is_invocable_r_v<Ret, decltype(Function), Args...>
    static Callback<Ret, Args...> from() {
        return {nullptr, []([[maybe_unused]] const void* obj, Args... args) -> Ret {
                    return Function(std::forward<Args>(args)...);
                }};
    }
};
