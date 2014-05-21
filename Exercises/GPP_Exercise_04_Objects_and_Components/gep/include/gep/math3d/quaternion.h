#pragma once

#include "gep/math3d/mat3.h"
#include "gep/math3d/mat4.h"

#include "gep/interfaces/scripting.h"

namespace gep
{
    template <typename T>
    struct Quaternion_t
    {
        union
        {
            struct
            {
                T x,y,z,angle;
            };
            T data[4];
        };
        typedef T component_t;

        /// \brief default constructor
        Quaternion_t()
        {
            this->x = 0;
            this->y = 0;
            this->z = 0;
            this->angle = 1;
        }

        /// \brief constructor without initialization
        Quaternion_t(DoNotInitialize arg) {}

        /// \brief constructor
        /// \param axis
        ///   the rotation axis
        /// \param angle
        ///   the amount of rotation around axis (in degrees)
        Quaternion_t(vec3_t<T> axis, T angle)
        {
            angle = toRadians(angle) / 2;
            float temp = sin(angle);

            this->x = axis.x * temp;
            this->y = axis.y * temp;
            this->z = axis.z * temp;
            this->angle = cos(angle);
        }

        /**
        * constructs a quaternion from a rotation matrix
        */
        Quaternion_t(mat3_t<T> rot){
            T trace = 1.0f + rot.data[0] + rot.data[4] + rot.data[8];
            if(trace > GetEpsilon<T>::value())
            {
                T S = gep::sqrt(trace) * 2.0f;
                this->x = ( rot.data[7] - rot.data[5] ) / S;
                this->y = ( rot.data[2] - rot.data[6] ) / S;
                this->z = ( rot.data[3] - rot.data[1] ) / S;
                this->angle = T(0.25) * S;
            }
            else
            {
                if( rot.data[0] > rot.data[4] && rot.data[0] > rot.data[8] ) //Column 0:
                {
                    T S = sqrt( 1.0f + rot.data[0] - rot.data[4] - rot.data[8] ) * 2;
                    this->x = 0.25f * S;
                    this->y = ( rot.data[3] + rot.data[1] ) / S;
                    this->z = ( rot.data[2] + rot.data[6] ) / S;
                    this->angle = ( rot.data[7] - rot.data[5] ) / S;
                }
                else if( rot.data[4] > rot.data[8] ) // Column 1:
                {
                    float S = sqrt( 1.0f + rot.data[4] - rot.data[0] - rot.data[8] ) * 2.0f;
                    this->x = ( rot.data[3] + rot.data[1] ) / S;
                    this->y = 0.25f * S;
                    this->z = ( rot.data[7] + rot.data[5] ) / S;
                    this->angle = ( rot.data[2] - rot.data[6] ) / S;
                }
                else
                {
                    float S = sqrt( 1.0f + rot.data[8] - rot.data[0] - rot.data[4] ) * 2.0f;
                    this->x = ( rot.data[2] + rot.data[6] ) / S;
                    this->y = ( rot.data[7] + rot.data[5] ) / S;
                    this->z = 0.25f * S;
                    this->angle = ( rot.data[3] - rot.data[1] ) / S;
                }
            }
        }

        /// \brief returns the normalized quaternion
        const Quaternion_t<T> normalized() const
        {
            Quaternion_t<T> res(DO_NOT_INITIALIZE);
            auto length = gep::sqrt(x * x + y * y + z * z + angle * angle);
            if(length != 0){
                res.x = x / length;
                res.y = y / length;
                res.z = z / length;
                res.angle = angle / length;
            }
            return res;
        }

        /// \brief returns the inverse of this quaternion
        const Quaternion_t<T> inverse() const
        {
            Quaternion_t<T> res(DO_NOT_INITIALIZE);
            res.x = x * -1;
            res.y = y * -1;
            res.z = z * -1;
            res.angle = angle;
            return res;
        }

        /// \brief converts this quaternion into a 4x4 rotation matrix
        const mat4_t<T> toMat4() const
        {
            GEP_ASSERT(isValid(), "quaternion is not valid");
            mat4_t<T> mat(DO_NOT_INITIALIZE);
            Quaternion norm = normalized();
            auto xx  = norm.x * norm.x;
            auto xy  = norm.x * norm.y;
            auto xz  = norm.x * norm.z;
            auto xw  = norm.x * norm.angle;
            auto yy  = norm.y * norm.y;
            auto yz  = norm.y * norm.z;
            auto yw  = norm.y * norm.angle;
            auto zz  = norm.z * norm.z;
            auto zw  = norm.z * norm.angle;
            mat.data[0]  = T(1) - T(2) * ( yy + zz );
            mat.data[1]  =        T(2) * ( xy - zw );
            mat.data[2]  =        T(2) * ( xz + yw );
            mat.data[4]  =        T(2) * ( xy + zw );
            mat.data[5]  = T(1) - T(2) * ( xx + zz );
            mat.data[6]  =        T(2) * ( yz - xw );
            mat.data[8]  =        T(2) * ( xz - yw );
            mat.data[9]  =        T(2) * ( yz + xw );
            mat.data[10] = T(1) - T(2) * ( xx + yy );
            mat.data[3]  = mat.data[7] = mat.data[11] = mat.data[12] = mat.data[13] = mat.data[14] = T(0);
            mat.data[15] = T(1);
            return mat;
        }

        /// \brief converts this quaternion into a 3x3 rotation matrix
        const mat3_t<T> toMat3() const
        {
            mat3_t<T> mat(DO_NOT_INITIALIZE);
            auto norm = this->normalized();
            auto xx  = norm.x * norm.x;
            auto xy  = norm.x * norm.y;
            auto xz  = norm.x * norm.z;
            auto xw  = norm.x * norm.angle;
            auto yy  = norm.y * norm.y;
            auto yz  = norm.y * norm.z;
            auto yw  = norm.y * norm.angle;
            auto zz  = norm.z * norm.z;
            auto zw  = norm.z * norm.angle;
            mat.data[0] = T(1) - T(2) * ( yy + zz );
            mat.data[1] =        T(2) * ( xy - zw );
            mat.data[2] =        T(2) * ( xz + yw );
            mat.data[3] =        T(2) * ( xy + zw );
            mat.data[4] = T(1) - T(2) * ( xx + zz );
            mat.data[5] =        T(2) * ( yz - xw );
            mat.data[6] =        T(2) * ( xz - yw );
            mat.data[7] =        T(2) * ( yz + xw );
            mat.data[8] = T(1) - T(2) * ( xx + yy );
            return mat;
        }

        /// \brief * operator for muliplying two quaternions
        const Quaternion_t<T> operator * (const Quaternion_t<T>& rh) const
        {
            Quaternion res(DO_NOT_INITIALIZE);
            res.x     = this->angle * rh.x     + this->x * rh.angle + this->y * rh.z - this->z * rh.y;
            res.y     = this->angle * rh.y     + this->y * rh.angle + this->z * rh.x - this->x * rh.z;
            res.z     = this->angle * rh.z     + this->z * rh.angle + this->x * rh.y - this->y * rh.x;
            res.angle = this->angle * rh.angle - this->x * rh.x     - this->y * rh.y - this->z * rh.z;
            return res;
        }

        /// \brief * operator for multiplying with a scalar
        const Quaternion_t<T> operator * (const float rh) const
        {
            Quaternion_t<T> res(DO_NOT_INITIALIZE);
            res.x = this->x * rh;
            res.y = this->y * rh;
            res.z = this->z * rh;
            res.angle = this->angle * rh;
            return res;
        }

        /// \brief checks if this quaternion is valid or not
        bool isValid() const
        {
            //nan checks
            return (!(x != x) && !(y != y) && !(z != z) && !(angle != angle)
                /*&& x != float.infinity && y != float.infinity && z != float.infinity && angle != float.infinity*/);
        }

        const Quaternion_t<T> Integrate(const vec3_t<T>& angularVelocity, T deltaTime) const
        {
            Quaternion_t<T> deltaQ(DO_NOT_INITIALIZE);
            auto scaledAngularVelocity = angularVelocity * (deltaTime * T(0.5));
            auto squaredVelocityLength = scaledAngularVelocity.squaredLength();

            T s = 1;

            if(squaredVelocityLength * squaredVelocityLength / T(24) < FloatEpsilon)
            {
                deltaQ.angle = T(1) - squaredVelocityLength / T(2);
                s = T(1) - squaredVelocityLength / T(6);
            }
            else
            {
                auto velocityLength = gep::sqrt(squaredVelocityLength);

                deltaQ.angle = gep::cos(velocityLength);
                s = gep::sin(velocityLength) / velocityLength;
            }

            deltaQ.x = scaledAngularVelocity.x * s;
            deltaQ.y = scaledAngularVelocity.y * s;
            deltaQ.z = scaledAngularVelocity.z * s;

            return deltaQ * (*this);
        }
        LUA_BIND_VALUE_TYPE_BEGIN
            LUA_BIND_FUNCTION(normalized)
            LUA_BIND_FUNCTION(inverse)
            //LUA_BIND_FUNCTION(toMat3)
            //LUA_BIND_FUNCTION(toMat4)
            LUA_BIND_FUNCTION(isValid)
        LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(x)
            LUA_BIND_MEMBER(y)
            LUA_BIND_MEMBER(z)
            LUA_BIND_MEMBER(angle)
        LUA_BIND_VALUE_TYPE_END
    };

    typedef Quaternion_t<float> Quaternion;
};
