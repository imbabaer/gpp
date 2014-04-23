#pragma once

#include <cmath>
#include "gep/math3d/constants.h"
#include "gep/math3d/algorithm.h"

#include "gep/interfaces/scripting.h"

namespace gep
{
    /// \brief A vector with two components, T defines the storage type for the two components
    template<typename T>
    struct vec2_t
    {
        union
        {
            struct
            {
                T x,y;
            };
            T data[2];
        };

        typedef T component_t;
        static const size_t dimension = 2;

        /// \brief default constructor
        inline vec2_t()
        {
            this->x = 0;
            this->y = 0;
        }

        /// \brief constructor for a unitialized instance
        inline vec2_t(DoNotInitialize) {}

        /// \brief constructor
        inline vec2_t(T x, T y)
        {
            this->x = x;
            this->y = y;
        }

        /// \brief constructor
        inline explicit vec2_t(T f[2])
        {
            this->x = f[0];
            this->y = f[1];
        }

        /// \brief constructor
        inline explicit vec2_t(const T xy)
        {
            this->x = xy;
            this->y = xy;
        }

        /// \brief + operator for adding to another 2 component vector
        inline const vec2_t<T> operator + (const vec2_t<T>& rh) const
        {
            return vec2_t<T>(this->x + rh.x, this->y + rh.y);
        }

        /// \brief - operator for adding to another 2 component vector
        inline const vec2_t<T> operator - (const vec2_t<T>& rh) const
        {
            return vec2_t<T>(this->x - rh.x, this->y - rh.y);
        }

        /// \brief * operator for multipling with a scalar
        inline const vec2_t<T> operator * (const T rh) const
        {
            return vec2_t<T>(this->x * rh, this->y * rh);
        }

        /// \brief * operator for multipliying component wise with another 2 component vector
        inline const vec2_t<T> operator * (const vec2_t<T>& rh) const
        {
            return vec2_t<T>(this->x * rh.x, this->y * rh.y);
        }

        /// \brief / operator for dividing through a scalar
        inline const vec2_t<T> operator / (const T rh) const
        {
            return vec2_t<T>(this->x / rh, this->y / rh);
        }

        /// \brief / operator for dividing component wise by another 2 component vector
        inline const vec2_t<T> operator / (const vec2_t<T>& rh) const
        {
            return vec2_t<T>(this->x / rh.x, this->y / rh.y);
        }

        /// \brief += operator
        inline vec2_t<T>& operator += (const vec2_t<T>& rh)
        {
            this->x += rh.x;
            this->y += rh.y;
            return *this;
        }

        /// \brief -= operator
        inline vec2_t<T>& operator -= (const vec2_t<T>& rh)
        {
            this->x -= rh.x;
            this->y -= rh.y;
            return *this;
        }

        /// \brief *= operator (scalar)
        inline vec2_t<T>& operator *= (const T rh)
        {
            this->x *= rh;
            this->y *= rh;
            return *this;
        }

        /// \brief *= operator (component vise)
        inline vec2_t<T>& operator *= (const vec2_t<T>& rh)
        {
            this->x *= rh.x;
            this->y *= rh.y;
            return *this;
        }

        /// \brief /= operator (scalar)
        inline vec2_t<T>& operator /= (const T rh)
        {
            this->x /= rh;
            this->y /= rh;
            return *this;
        }

        /// \brief /= operator (component-vise)
        inline vec2_t<T>& operator /= (const vec2_t<T>& rh)
        {
            this->x /= rh.x;
            this->y /= rh.y;
            return *this;
        }

        /// \brief unary - operator
        inline const vec2_t<T> operator - () const
        {
            return vec2_t<T>(-x, -y);
        }

        /// \brief comparing two instances with an epsilon
        inline bool epsilonCompare (const vec2_t<T>& rh, const T e = GetEpsilon<T>::value()) const
        {
            return ( (this->x - e <= rh.x && this->x + e >= rh.x) &&
                (this->y - e <= rh.y && this->y + e >= rh.y) );
        }

        /// \brief computes the dot product of this and another 2 component vector
        inline T dot(const vec2_t<T>& rh) const
        {
            return this->x * rh.x + this->y * rh.y;
        }

        /// \brief computes the length of this vector
        inline typename CalcSqrt<T>::result_t length() const
        {
            return CalcSqrt<T>::sqrt(x*x + y*y);
        }

        /// \brief computes the squared length of this vector
        inline T squaredLength() const
        {
            return x*x + y*y;
        }

        /// \brief returns the normalized vector
        inline const vec2_t<T> normalized() const
        {
            T length = this->length();
            vec2_t<T> res = *this;
            if(length != 0){
                res = res / length;
            }
            return res;
        }

        LUA_BIND_VALUE_TYPE_BEGIN
            LUA_BIND_FUNCTION(length)
            LUA_BIND_FUNCTION(squaredLength)
        LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(x)
            LUA_BIND_MEMBER(y)
         LUA_BIND_VALUE_TYPE_END
    };

    typedef vec2_t<float> vec2;
    typedef vec2_t<int> ivec2;
};
