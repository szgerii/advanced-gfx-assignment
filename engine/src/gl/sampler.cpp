#include "gl/sampler.h"

#define SAMPLER_PARAM_IMPL(pname, penum, ptype, suffix)                                            \
    void Sampler::set_##pname(ptype pname) {                                                       \
        params.pname = (pname);                                                                    \
        glSamplerParameter##suffix(id_, (penum), (pname));                                         \
    }

#define SAMPLER_PARAM_IMPL_INT(pname, penum) SAMPLER_PARAM_IMPL(pname, penum, GLint, i)
#define SAMPLER_PARAM_IMPL_FLOAT(pname, penum) SAMPLER_PARAM_IMPL(pname, penum, GLfloat, f)

SAMPLER_PARAM_IMPL_INT(min_filter, GL_TEXTURE_MIN_FILTER)
SAMPLER_PARAM_IMPL_INT(mag_filter, GL_TEXTURE_MAG_FILTER)
SAMPLER_PARAM_IMPL_INT(wrap_s, GL_TEXTURE_WRAP_S)
SAMPLER_PARAM_IMPL_INT(wrap_t, GL_TEXTURE_WRAP_T)
SAMPLER_PARAM_IMPL_INT(wrap_r, GL_TEXTURE_WRAP_R)
SAMPLER_PARAM_IMPL_INT(min_lod, GL_TEXTURE_MIN_LOD)
SAMPLER_PARAM_IMPL_INT(max_lod, GL_TEXTURE_MAX_LOD)
SAMPLER_PARAM_IMPL_FLOAT(lod_bias, GL_TEXTURE_LOD_BIAS)
SAMPLER_PARAM_IMPL_INT(compare_func, GL_TEXTURE_COMPARE_FUNC)
SAMPLER_PARAM_IMPL_INT(compare_mode, GL_TEXTURE_COMPARE_MODE)

#undef SAMPLER_PARAM_IMPL_FLOAT
#undef SAMPLER_PARAM_IMPL_INT
#undef SAMPLER_PARAM_IMPL

void Sampler::set_border_color(glm::vec4 border_color) {
    params.border_color = border_color;
    glSamplerParameterfv(id_, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(border_color));
}

void Sampler::set_filters(GLint min_filter, GLint mag_filter) {
    set_min_filter(min_filter);
    set_mag_filter(mag_filter);
}

void Sampler::set_wrap(GLint s, GLint t) {
    set_wrap_s(s);
    set_wrap_t(t);
}

void Sampler::set_wrap(GLint s, GLint t, GLint r) {
    set_wrap(s, t);
    set_wrap_r(r);
}

void Sampler::set_lod(GLint min_lod, GLint max_lod, GLfloat lod_bias) {
    set_min_lod(min_lod);
    set_max_lod(max_lod);
    set_lod_bias(lod_bias);
}

void Sampler::set_depth(GLint compare_func, GLint compare_mode) {
    set_compare_func(compare_func);
    set_compare_mode(compare_mode);
}

void Sampler::update_parameters() {
    // ehh, but works
    set_filters(params.min_filter, params.mag_filter);
    set_wrap(params.wrap_s, params.wrap_t, params.wrap_r);
    set_lod(params.min_lod, params.max_lod, params.lod_bias);
    set_depth(params.compare_func, params.compare_mode);
    set_border_color(params.border_color);
}
