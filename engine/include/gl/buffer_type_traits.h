#pragma once

#include <string_view>

#include "vendor.h"

// NOTE: for now, only index bindable buffer types have proper type trait support

#define DEFINE_TYPE_TRAIT_VALUES(idx_bindable_val, max_bindings_pname_val, type_name_val)          \
    static constexpr bool idx_bindable          = idx_bindable_val;                                \
    static constexpr GLenum max_bindings_pname  = max_bindings_pname_val;                          \
    static constexpr std::string_view type_name = type_name_val;

// separated into concept to be able to constrain buffer_type_traits<GLenum> to valid buffer types
template <GLenum BufferType>
concept CIsBufferType =
    BufferType == GL_ARRAY_BUFFER || BufferType == GL_ATOMIC_COUNTER_BUFFER ||
    BufferType == GL_COPY_READ_BUFFER || BufferType == GL_COPY_WRITE_BUFFER ||
    BufferType == GL_DISPATCH_INDIRECT_BUFFER || BufferType == GL_DRAW_INDIRECT_BUFFER ||
    BufferType == GL_ELEMENT_ARRAY_BUFFER || BufferType == GL_PIXEL_PACK_BUFFER ||
    BufferType == GL_PIXEL_UNPACK_BUFFER || BufferType == GL_QUERY_BUFFER ||
    BufferType == GL_SHADER_STORAGE_BUFFER || BufferType == GL_TEXTURE_BUFFER ||
    BufferType == GL_TRANSFORM_FEEDBACK_BUFFER || BufferType == GL_UNIFORM_BUFFER;

template <GLenum BufferType>
requires CIsBufferType<BufferType>
struct buffer_type_traits {
    DEFINE_TYPE_TRAIT_VALUES(false, 0, "UNKNOWN")
};

template <>
struct buffer_type_traits<GL_ATOMIC_COUNTER_BUFFER> {
    DEFINE_TYPE_TRAIT_VALUES(true, GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS, "ATOMIC COUNTER BUFFER")
};

template <>
struct buffer_type_traits<GL_TRANSFORM_FEEDBACK_BUFFER> {
    DEFINE_TYPE_TRAIT_VALUES(true, GL_MAX_TRANSFORM_FEEDBACK_BUFFERS, "TRANSFORM FEEDBACK BUFFER")
};

template <>
struct buffer_type_traits<GL_UNIFORM_BUFFER> {
    DEFINE_TYPE_TRAIT_VALUES(true, GL_MAX_UNIFORM_BUFFER_BINDINGS, "UNIFORM BUFFER")
};

template <>
struct buffer_type_traits<GL_SHADER_STORAGE_BUFFER> {
    DEFINE_TYPE_TRAIT_VALUES(true, GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, "SHADER STORAGE BUFFER")
};

template <GLenum BufferType>
concept CCanBindBase = CIsBufferType<BufferType> && buffer_type_traits<BufferType>::idx_bindable;
