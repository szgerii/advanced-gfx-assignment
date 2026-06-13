#pragma once

#include <type_traits>

// it's recommended to turn this off even in debug builds after everything has been set up
// VALIDATE is meant to be a place for crude invariant checking
#ifndef NDEBUG
    #define OGL_APP_ENABLE_VALIDATE 1
#endif

#ifdef OGL_APP_ENABLE_VALIDATE
    #define DEFINE_VALIDATOR(body)                                                                 \
        void validate() const {                                                                    \
            body                                                                                   \
        }
#else
    #define DEFINE_VALIDATOR(body)
#endif

#ifdef OGL_APP_ENABLE_VALIDATE
    #define VALIDATE(obj) (obj).validate()
#else
    #define VALIDATE(obj)
#endif

#ifdef OGL_APP_ENABLE_VALIDATE
    #define VALIDATE_ALL(coll)                                                                     \
        for (const auto& it : (coll)) {                                                            \
            VALIDATE(it);                                                                          \
        }
#else
    #define VALIDATE_ALL(coll)
#endif

#ifdef OGL_APP_ENABLE_VALIDATE
    #define VALIDATE_ENUM_MSG(T, value, msg)                                                       \
        static_assert(std::is_enum_v<T>, "VALIDATE_ENUM called with non-enum type");               \
        static_assert(T::FIRST == T::FIRST && T::LAST == T::LAST,                                  \
                      "VALIDATE_ENUM on enum with untagged first and last value");                 \
        assert(T::FIRST <= (value) && (value) <= T::LAST && (msg));
    #define VALIDATE_ENUM(T, value)                                                                \
        VALIDATE_ENUM_MSG(T, value, "underlying enum value out of bounds");
#else
    #define VALIDATE_ENUM_MSG(T, value, msg)
    #define VALIDATE_ENUM(T, value)
#endif
