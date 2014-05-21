#include "stdafx.h"
#include "gep/unittest/UnittestManager.h"
#include "gep/math3d/vec2.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/mat3.h"
#include "gep/math3d/mat4.h"
#include "gep/math3d/quaternion.h"
#include "gep/math3d/algorithm.h"
#include "gep/math3d/aabb.h"
#include "gep/math3d/sphere.h"
#include "gep/math3d/ray.h"
#include "gep/math3d/plane.h"

using namespace gep;

GEP_UNITTEST_GROUP(math3d)
GEP_UNITTEST_TEST(math3d, vec2)
{
    {
      vec2 temp;
      GEP_ASSERT(temp.epsilonCompare(vec2(0.0f)), "default initialization not correct");
    }

    {
      vec2 temp(2.0f, 0.0f);
      GEP_ASSERT(temp.normalized().epsilonCompare(vec2(1.0f, 0.0f)));
    }
}

GEP_UNITTEST_TEST(math3d, vec3)
{
    static_assert(sizeof(vec3) == sizeof(float)*3, "a vec3 has to contain exactly 3 floats");

    //test default constructor
    {
        vec3 temp;
        GEP_ASSERT(temp.x == 0.0f && temp.y == 0.0f && temp.z == 0.0f, "default initialization not correct");
    }

    // test no initialize constructor
    {
        float mem[sizeof(vec3)/sizeof(float)];
        mem[0] = 1.0f;
        mem[1] = 2.0f;
        mem[2] = 3.0f;
        vec3* temp = new (mem) vec3(DO_NOT_INITIALIZE);
        GEP_ASSERT(temp->x == 1.0f && temp->y == 2.0f && temp->z == 3.0f, "no initialize constructor of vec3 does not work");
    }

    // test 3 paremeter constructor
    {
        vec3 temp(1.0f, 2.0f, 3.0f);
        GEP_ASSERT(temp.x == 1.0f && temp.y == 2.0f && temp.z == 3.0f, "3 parameter constructor not correct");
    }

    // test construction from static array
    {
        float initWith[] = { 4.0f, 5.0f, 6.0f };
        vec3 temp(initWith);
        GEP_ASSERT(temp.x == 4.0f && temp.y == 5.0f && temp.z == 6.0f, "constructon from static array not correct");
    }

    // test construction from single value
    {
        vec3 temp(5.0f);
        GEP_ASSERT(temp.x == 5.0f && temp.y == 5.0f && temp.z == 5.0f, "constructon from single value not correct");
    }

    // test construction from vec2 and third value
    {
        vec3 temp(vec2(3.0f, 2.0f), 1.0f);
        GEP_ASSERT(temp.x == 3.0f && temp.y == 2.0f && temp.z == 1.0f, "construction from vec2 and additional value not correct");
    }

    // test operator +
    {
        vec3 result = vec3(1, 2, 3) + vec3(4, 5, 6);
        GEP_ASSERT(epsilonCompare(result, vec3(5.0f, 7.0f, 9.0f)), "vec3 operator + not correct");
    }

    // test operator -
    {
        vec3 result = vec3(4, 5, 6) - vec3(3, 1, 2);
        GEP_ASSERT(epsilonCompare(result, vec3(1.0f, 4.0f, 4.0f)), "vec3 operator - not correct");
    }

    // test operator * with scalar
    {
        vec3 result = vec3(1, 2, 3) * 2.0f;
        GEP_ASSERT(epsilonCompare(result, vec3(2.0f, 4.0f, 6.0f)), "vec3 operator * with scalar not correct");
    }

    // test operator *
    {
        vec3 result = vec3(1, 2, 3) * vec3(2, 3, 4);
        GEP_ASSERT(epsilonCompare(result, vec3(2.0f, 6.0f, 12.0f)), "vec3 operator * not correct");
    }

    // test operator / with scalar
    {
        vec3 result = vec3(1, 2, 3) / 2.0f;
        GEP_ASSERT(epsilonCompare(result, vec3(0.5f, 1.0f, 1.5f)), "vec3 operator / with scalar not correct");
    }

    // test operator /
    {
        vec3 result = vec3(8, 6, 4) / vec3(4, 3, 2);
        GEP_ASSERT(epsilonCompare(result, vec3(2.0f, 2.0f, 2.0f)), "vec3 operator / not correct");
    }

    // test operator +=
    {
        vec3 result(1, 2, 3);
        result += vec3(4, 5, 6);
        GEP_ASSERT(epsilonCompare(result, vec3(5.0f, 7.0f, 9.0f)), "vec3 operator += not correct");
    }

    // test operator -=
    {
        vec3 result(1, 2, 3);
        result -= vec3(4, 5, 6);
        GEP_ASSERT(epsilonCompare(result, vec3(-3.0f, -3.0f, -3.0f)), "vec3 operator -= not correct");
    }

    // test operator *= with scalar
    {
        vec3 result(1, 2, 3);
        result *= 4.0f;
        GEP_ASSERT(epsilonCompare(result, vec3(4.0f, 8.0f, 12.0f)), "vec3 operator *= with scalar not correct");
    }

    // test operator *=
    {
        vec3 result(1, 2, 3);
        result *= vec3(4, 5, 6);
        GEP_ASSERT(epsilonCompare(result, vec3(4.0f, 10.0f, 18.0f)), "vec3 operator *= not correct");
    }

    // test operator /= with scalar
    {
        vec3 result(1, 2, 3);
        result /= 2.0f;
        GEP_ASSERT(epsilonCompare(result, vec3(0.5f, 1.0f, 1.5f)), "vec3 operator /= with scalar not correct");
    }

    // test operator /=
    {
        vec3 result(1, 2, 3);
        result /= vec3(1, 2, 3);
        GEP_ASSERT(epsilonCompare(result, vec3(1.0f, 1.0f, 1.0f)), "vec3 operator /= not correct");
    }

    // test unary -
    {
        vec3 result(1, 2, 3);
        GEP_ASSERT(epsilonCompare(-result, vec3(-1.0f, -2.0f, -3.0f)), "vec3 unary - not correct");
    }

    // test epsilon compare
    {
        GEP_ASSERT(vec3(1, 2, 3).epsilonCompare(vec3(1, 2, 3)) == true, "vec3 epsilon compare does not work");
        GEP_ASSERT(vec3(1, 2, 3).epsilonCompare(vec3(1.1f, 2.1f, 3.1f), 0.101f) == true, "vec3 epsilon compare does not work");
        GEP_ASSERT(vec3(1, 2, 3).epsilonCompare(vec3(1.1f, 2.1f, 3.1f), 0.09f) == false, "vec3 epslion compare does not work");
    }

    // test length
    {
        GEP_ASSERT(epsilonCompare(vec3(2, 0, 0).length(), 2.0f), "vec3 length does not work correctly");
        GEP_ASSERT(epsilonCompare(vec3(0, 2, 0).length(), 2.0f), "vec3 length does not work correctly");
        GEP_ASSERT(epsilonCompare(vec3(0, 0, 2).length(), 2.0f), "vec3 length does not work correctly");
    }

    // test squared length
    {
        GEP_ASSERT(epsilonCompare(vec3(2, 4, 6).squaredLength(), 2.0f*2.0f + 4.0f*4.0f + 6.0f*6.0f), "vec3 squaredLength does not work correctly");
    }

    // test dot product
    {
        GEP_ASSERT(epsilonCompare(vec3(1, 2, 3).dot(vec3(4, 5, 6)), 32.0f), "vec3 dot does not work correctly");
    }

    // test cross product
    {
        GEP_ASSERT(epsilonCompare(vec3(1, 0, 0).cross(vec3(0, 1, 0)), vec3(0, 0, 1)), "vec3 cross does not work correctly");
    }

    // test normalized
    {
        GEP_ASSERT(epsilonCompare(vec3(1, 2, 3).normalized().length(), 1.0f), "vec3 normalized does not work correctly");
    }
}

GEP_UNITTEST_TEST(math3d, mat3)
{
    // test mat3 inverse
    {
        mat3 scale;
        scale.data[0] = 5.0f;
        scale.data[4] = 5.0f;
        scale.data[8] = 5.0f;

        mat3 inverseScale = scale.inverse();
        mat3 identity = scale * inverseScale;
        GEP_ASSERT(epsilonCompare(identity, mat3::identity()), "mat3 inverse not correct", inverseScale.m00);
    }

    // test mat3 identity
    {
        mat3 identity = mat3::identity();
        float shouldBe[] = {1.0f, 0.0f, 0.0f,
                            0.0f, 1.0f, 0.0f,
                            0.0f, 0.0f, 1.0f};
        for(int i=0; i< GEP_ARRAY_SIZE(shouldBe); i++)
            GEP_ASSERT(identity.data[i] == shouldBe[i], "mat3 identity matrix is not correct", i);
    }

    // test mat3 * vec3
    {
        vec3 rotated = mat3::rotationMatrixXYZ(vec3(10,20,30)) * vec3(1,2,3);
        GEP_ASSERT(epsilonCompare(rotated, vec3(0.735318542f, 1.81794405f, 3.18659449f)), "mat3 vec3 multiplication does not work correctly");
    }

    // test mat3 * mat3 multiplication
    {
        mat3 rotMat = mat3::rotationMatrixXYZ(vec3(10, 20, 30));
        mat3 scaleMatrix = mat3::scaleMatrix(vec3(1, 2, 3));
        vec3 vector(4,3,2);

        vec3 result1 = (rotMat * scaleMatrix) * vector;
        vec3 result2 = scaleMatrix * vector;
        result2 = rotMat * result2;
        GEP_ASSERT(epsilonCompare(result1, result2), "mat3 mat3 multiplication does not work. Maybe the order is inverse?");
    }
}

GEP_UNITTEST_TEST(math3d, mat4)
{
    // test mat4 inverse
    {
        mat4 scale;
        scale.m00 = 5.0f;
        scale.m11 = 5.0f;
        scale.m22 = 5.0f;
        scale.m33 = 5.0f;

        mat4 inverseScale = scale.inverse();
        mat4 identity = scale * inverseScale;
        GEP_ASSERT(epsilonCompare(identity, mat4::identity()), "mat4 inverse not correct", inverseScale.m00);
    }

    // test mat4 identity
    {
        mat4 identity = mat4::identity();
        float shouldBe[] = {1.0f, 0.0f, 0.0f, 0.0f,
                            0.0f, 1.0f, 0.0f, 0.0f,
                            0.0f, 0.0f, 1.0f, 0.0f,
                            0.0f, 0.0f, 0.0f, 1.0f};
        for(int i=0; i< GEP_ARRAY_SIZE(shouldBe); i++)
            GEP_ASSERT(identity.data[i] == shouldBe[i], "mat4 identity matrix is not correct", i);
    }

    // test mat4 multiplication with direction
    {
        vec3 vector(1, 2, 3);

        vec3 result1 = mat4::translationMatrix(vec3(4, 5, 6)).transformDirection(vector);
        GEP_ASSERT(epsilonCompare(result1, vector), "multiplying a mat4 with a direction is not correct. Affine part should not be used.");

        vec3 result2 = mat4::rotationMatrixXYZ(vec3(10, 20, 30)).transformDirection(vector);
        GEP_ASSERT(epsilonCompare(result2, vec3(0.735318542f, 1.81794405f, 3.18659449f)), "multiplying a mat4 with a direction does not correctly rotate");
    }

    // test mat4 multiplication with position
    {
        vec3 vector(1, 2, 3);

        vec3 result1 = mat4::translationMatrix(vec3(4, 5, 6)).transformPosition(vector);
        GEP_ASSERT(epsilonCompare(result1, vec3(5, 7, 9)), "multiplying a mat4 with a position is not correct. Affine part is not used correctly");

        vec3 result2 = mat4::rotationMatrixXYZ(vec3(10, 20, 30)).transformPosition(vector);
        GEP_ASSERT(epsilonCompare(result2, vec3(0.735318542f, 1.81794405f, 3.18659449f)), "multiplying a mat4 with a position does not correctly rotate");
    }

    // test mat4 with mat4 multiplication
    {
        vec3 vector(1, 2, 3);
        mat4 rotMat = mat4::rotationMatrixXYZ(vec3(10, 20, 30));
        mat4 transMat = mat4::translationMatrix(vec3(4, 5, 6));

        vec3 result1 = (transMat * rotMat).transformPosition(vector);
        vec3 result2 = transMat.transformPosition(rotMat.transformPosition(vector));
        GEP_ASSERT(epsilonCompare(result1, result2), "mat4 * mat4 is incorrect. Maybe the order is inverse?");
    }
}

GEP_UNITTEST_TEST(math3d, aabb)
{
    // test aabb vertices
    {
        //2d case
        AxisAlignedBox_t<vec2> aabb2d(vec2(-2, -2), vec2(1, 1));
        vec2 vertices2D[4];
        aabb2d.getVertices(vertices2D);
        GEP_ASSERT(epsilonCompare(vertices2D[0], vec2(-2, -2)));
        GEP_ASSERT(epsilonCompare(vertices2D[1], vec2( 1, -2)));
        GEP_ASSERT(epsilonCompare(vertices2D[2], vec2(-2,  1)));
        GEP_ASSERT(epsilonCompare(vertices2D[3], vec2( 1,  1)));

        //3d case
        AxisAlignedBox_t<vec3> aabb3d(vec3(-1, -2, -3), vec3(4, 5, 6));
        vec3 vertices3D[8];
        aabb3d.getVertices(vertices3D);
        GEP_ASSERT(epsilonCompare(vertices3D[0], vec3(-1, -2, -3)));
        GEP_ASSERT(epsilonCompare(vertices3D[1], vec3( 4, -2, -3)));
        GEP_ASSERT(epsilonCompare(vertices3D[2], vec3(-1,  5, -3)));
        GEP_ASSERT(epsilonCompare(vertices3D[3], vec3( 4,  5, -3)));
        GEP_ASSERT(epsilonCompare(vertices3D[4], vec3(-1, -2,  6)));
        GEP_ASSERT(epsilonCompare(vertices3D[5], vec3( 4, -2,  6)));
        GEP_ASSERT(epsilonCompare(vertices3D[6], vec3(-1,  5,  6)));
        GEP_ASSERT(epsilonCompare(vertices3D[7], vec3( 4,  5,  6)));
    }

    //test intersects and contains methods
    {
        AABB box1 = AABB(vec3(-20,-20,-20),vec3(-10,-10,-10));
        AABB box2 = AABB(vec3(10,10,10),vec3(20,20,20));
        GEP_ASSERT(box1.intersects(box2) == false);

        AABB box3 = AABB(vec3(-10,-10,-10),vec3(10,10,10));
        AABB box4 = AABB(vec3(-20,-5,-5),vec3(20,5,5));
        AABB box5 = AABB(vec3(-5,-20,-5),vec3(5,20,5));
        AABB box6 = AABB(vec3(-5,-5,-20),vec3(5,5,20));
        AABB box7 = AABB(vec3(-5,-5,-5),vec3(5,5,5));
        GEP_ASSERT(box3.intersects(box3));
        GEP_ASSERT(box3.intersects(box4) && box4.intersects(box3));
        GEP_ASSERT(box3.intersects(box5) && box5.intersects(box3));
        GEP_ASSERT(box3.intersects(box6) && box6.intersects(box3));
        GEP_ASSERT(box3.intersects(box7) && box7.intersects(box3));

        GEP_ASSERT((box3.contains(box7)) == true);
        GEP_ASSERT((box5.contains(box4)) == false);
    }
}

GEP_UNITTEST_TEST(math3d, sphere)
{
    auto r = Ray(vec3(0,0,0), vec3(2.9f,3.1f,3.2f).normalized());
    auto s = Sphere(vec3(3,3,3), 1.0f);
    GEP_ASSERT(s.intersects(r) == true);
}

GEP_UNITTEST_TEST(math3d, plane)
{
    Plane p1 = Plane(vec3(0,0,0),vec3(1,0,0));
    Plane p2 = Plane(vec3(0,0,0),vec3(0,1,0));

    Ray result = p1.intersectWith(p2);
    GEP_ASSERT(result.pos.epsilonCompare(vec3(0.0f, 0.0f, 0.0f)));
    GEP_ASSERT(result.dir.epsilonCompare(vec3(0.0f, 0.0f, 1.0f)));

    Plane p3 = Plane(0.0f, 1.0f, 0.0f, 0.5f);
    Plane p4 = Plane(0.0f, 0.0f, 1.0f, 1.0f);
    Ray result2 = p3.intersectWith(p4);
    GEP_ASSERT(result2.pos.epsilonCompare(vec3(0.0f, 0.5f, 1.0f)));
    GEP_ASSERT(result2.dir.epsilonCompare(vec3(1.0f, 0.0f, 0.0f)));

    p1 = Plane(0,1,0,-1);
    p2 = Plane(1,0,0,1);
    result = p1.intersectWith(p2);
    GEP_ASSERT(result.pos.epsilonCompare(vec3(1,-1,0)));
    GEP_ASSERT(result.dir.epsilonCompare(vec3(0,0,-1)));

    p1 = Plane(0,0,1,-1);
    p2 = Plane(0,1,0,1);
    result = p1.intersectWith(p2);
    GEP_ASSERT(result.pos.epsilonCompare(vec3(0,1,-1)));
    GEP_ASSERT(result.dir.epsilonCompare(vec3(-1,0,0)));

    p1 = Plane(0,0,1,-1);
    p2 = Plane(1,0,0,1);
    result = p1.intersectWith(p2);
    GEP_ASSERT(result.pos.epsilonCompare(vec3(1,0,-1)));
    GEP_ASSERT(result.dir.epsilonCompare(vec3(0,1,0)));
}

GEP_UNITTEST_GROUP(algorithm)
GEP_UNITTEST_TEST(algorithm, floor)
{
    {
        float result = gep::floor(0.125f);
        GEP_ASSERT(result == 0.0f, "floor single value fail", result);
    }

    {
        vec2 result = gep::floor(vec2(0.125f, 1.1f));
        GEP_ASSERT(result.epsilonCompare(vec2(0.0f, 1.0f)), "floor vec2 fail", result.x, result.y);
    }

    {
        vec3 result = gep::floor(vec3(0.125f, 1.1f, -3.1f));
        GEP_ASSERT(result.epsilonCompare(vec3(0.0f, 1.0f, -4.0f)), "floor vec3 fail", result.x, result.y, result.z);
    }
}

GEP_UNITTEST_TEST(algorithm, ceil)
{
    {
        float result = gep::ceil(0.125f);
        GEP_ASSERT(result == 1.0f, "ceil single value fail", result);
    }

    {
        vec2 result = gep::ceil(vec2(0.125f, 1.1f));
        GEP_ASSERT(result.epsilonCompare(vec2(1.0f, 2.0f)), "ceil vec2 fail", result.x, result.y);
    }

    {
        vec3 result = gep::ceil(vec3(0.125f, 1.1f, -3.1f));
        GEP_ASSERT(result.epsilonCompare(vec3(1.0f, 2.0f, -3.0f)), "ceil vec3 fail", result.x, result.y, result.z);
    }
}

GEP_UNITTEST_TEST(math3d, Quaternion)
{
    {
        Quaternion q(vec3(1.0f, 0.0f, 0.0f), 35.0f);
        Quaternion noRot = q * q.inverse();
        GEP_ASSERT(epsilonCompare(noRot.x, 0.0f) && epsilonCompare(noRot.y, 0.0f) && epsilonCompare(noRot.z, 0.0f));
    }
}

GEP_UNITTEST_TEST(math3d, AABB)
{
    AABB box1 = AABB(vec3(-20,-20,-20),vec3(-10,-10,-10));
    AABB box2 = AABB(vec3(10,10,10),vec3(20,20,20));
    GEP_ASSERT(box1.intersects(box2) == false);

    AABB box3 = AABB(vec3(-10,-10,-10),vec3(10,10,10));
    AABB box4 = AABB(vec3(-20,-5,-5),vec3(20,5,5));
    AABB box5 = AABB(vec3(-5,-20,-5),vec3(5,20,5));
    AABB box6 = AABB(vec3(-5,-5,-20),vec3(5,5,20));
    AABB box7 = AABB(vec3(-5,-5,-5),vec3(5,5,5));
    GEP_ASSERT(box3.intersects(box3));
    GEP_ASSERT(box3.intersects(box4) && box4.intersects(box3));
    GEP_ASSERT(box3.intersects(box5) && box5.intersects(box3));
    GEP_ASSERT(box3.intersects(box6) && box6.intersects(box3));
    GEP_ASSERT(box3.intersects(box7) && box7.intersects(box3));

    GEP_ASSERT((box3.contains(box7)) == true);
    GEP_ASSERT((box4.contains(box5)) == false);

    AABB box8(vec3(-250,-250,-250),vec3(0,0,0));
    AABB box9(vec3(10.8533f,12.2839f,-23.83f), vec3(19.1467f,21.7161f,-14.17f));
    GEP_ASSERT(box9.intersects(box8) == false);
    GEP_ASSERT((box9.contains(box8)) == false);
    GEP_ASSERT(box7.contains(vec3(-4,4,2)) == true);
}
