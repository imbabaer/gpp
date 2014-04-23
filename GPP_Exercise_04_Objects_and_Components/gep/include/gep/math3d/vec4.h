#pragma once

namespace gep{
    template <typename T>
    struct vec4_t {
        union {
            struct {
                T x,y,z,w;
            };
            T data[4];
        };
    };
    typedef vec4_t<float> vec4;
}
