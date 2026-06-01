#pragma once

#include <type_traits>

#ifndef NDEBUG
    #define DEFINE_VALIDATOR(body)                                                                 \
        void validate() const {                                                                    \
            body                                                                                   \
        }
#else
    #define DEFINE_VALIDATOR(body)
#endif

#ifndef NDEBUG
    #define VALIDATE(obj) (obj).validate();
#else
    #define VALIDATE(obj)
#endif

#ifndef NDEBUG
    #define VALIDATE_ENUM(T, value, msg)                                                           \
        static_assert(std::is_enum_v<T>, "VALIDATE_ENUM called with non-enum type");               \
        static_assert(T::FIRST == T::FIRST && T::LAST == T::LAST,                                  \
                      "VALIDATE_ENUM on enum with untagged first and last value");                 \
        assert(T::FIRST <= (value) && (value) <= T::LAST && (msg));
#else
    #define VALIDATE_ENUM(T, value)
#endif
