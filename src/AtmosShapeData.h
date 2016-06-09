#pragma once

#include <string>

#ifndef FLOAT3
#define SIZE_FLOAT_3 3 * sizeof(float)

struct shapeData
{
    shapeData(std::string name):name(name), materialType(0){}

    // RTTI
    std::string name;

    // 0: Glass / 1: Mirror / 2: Diffuse
    int materialType;
};

struct diskData : public shapeData
{
    diskData():radius(0), shapeData("Disk")
    {
        memset(center, 0.0f, SIZE_FLOAT_3);
        memset(normal, 0.0f, SIZE_FLOAT_3);
    }

    float radius;
    float center[3], normal[3];
};

struct meshData : public shapeData
{
    meshData():supportKeyFrame(false), shapeData("Mesh")
    {
        strcpy(modelPath, "./data/models/test.obj");
    }

    char modelPath[1024];
    bool supportKeyFrame;
};

struct sphereData : public shapeData
{
    sphereData() :radius(0), shapeData("Sphere") 
    {
        memset(center, 0.0f, SIZE_FLOAT_3);
    }

    float radius;
    float center[3];
};

struct infinitePlaneData : public shapeData
{
    infinitePlaneData():shapeData("InfinitePlane")
    {
        memset(position, 0.0f, SIZE_FLOAT_3);
        memset(normal, 0.0f, SIZE_FLOAT_3);
    }

    float position[3], normal[3];
};

struct planeData : public shapeData
{
    planeData() :width(0.0f), height(0.0f), shapeData("Plane")
    {
        memset(position, 0.0f, SIZE_FLOAT_3);
        memset(normal, 0.0f, SIZE_FLOAT_3);
    }

    float position[3], normal[3];
    float width, height;
};

struct triangleData : public shapeData
{
    triangleData() :shapeData("Triangle")
    {
        memset(v0, 0.0f, SIZE_FLOAT_3);
        memset(v1, 0.0f, SIZE_FLOAT_3);
        memset(v2, 0.0f, SIZE_FLOAT_3);

        memset(vt0, 0.0f, SIZE_FLOAT_3);
        memset(vt1, 0.0f, SIZE_FLOAT_3);
        memset(vt2, 0.0f, SIZE_FLOAT_3);

        memset(n0, 0.0f, SIZE_FLOAT_3);
        memset(n1, 0.0f, SIZE_FLOAT_3);
        memset(n2, 0.0f, SIZE_FLOAT_3);
    }

    float v0[3], v1[3], v2[3];
    float vt0[3], vt1[3], vt2[3];
    float n0[3], n1[3], n2[3];
};

#undef FLOAT3

#endif
