#pragma once
#include <string>

#ifndef FLOAT3
#define SIZE_FLOAT_3 3 * sizeof(float)

struct lightData
{
    lightData(std::string name) :name(name) {}

    std::string name;
};

struct pointLightData : public lightData
{
    pointLightData() :lightData("Point Light")
    {
        memset(position, 0.0f, SIZE_FLOAT_3);
        memset(intensity, 0.0f, SIZE_FLOAT_3);
    }

    float position[3];
    float intensity[3];
};

struct spotLightData : public lightData
{
    spotLightData() :coneAngle(0.0f), falloffStart(0.0f), lightData("Spot Light")
    {
        memset(position, 0.0f, SIZE_FLOAT_3);
        memset(intensity, 0.0f, SIZE_FLOAT_3);
        memset(direction, 0.0f, SIZE_FLOAT_3);
    }

    float position[3], direction[3];

    float intensity[3];

    // 圆锥与中心线夹角(弧度)
    float coneAngle;

    // 开始产生半影的角度(弧度)
    float falloffStart;
};

struct areaLightData : public lightData
{
    areaLightData() :shapesType(0), lightData("Area Light")
    {
        memset(emission, 0.0f, SIZE_FLOAT_3);
    }

    float emission[3];

    int shapesType;
};

struct infiniteAreaLightData : public lightData
{
    infiniteAreaLightData() :lightData("Inifinite Area Light")
    {
        strcpy(imagePath, "./data/images/envmap.png");
    }

    char imagePath[1024];
};

#undef FLOAT3

#endif