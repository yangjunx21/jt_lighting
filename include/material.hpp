#ifndef MATERIAL_H
#define MATERIAL_H
#include <vecmath.h>

#include <cassert>
#include <iostream>

#include "hit.hpp"
#include "ray.hpp"
using std::cout;
using std::endl;
class Material {
   public:
    explicit Material(const Vector3f &color,
                      const Vector3f &s_color = Vector3f::ZERO, float s = 0,
                      const Vector3f &e_color = Vector3f::ZERO, float r = 1,
                      Vector3f t = Vector3f(1, 0, 0),
                      const char *textureFile = "", const char *bumpFile = "")
        : color(color),
          specularColor(s_color),
          shininess(s),
          emission(e_color),
          refr(r),
          type(t){}

    virtual ~Material() = default;

    Vector3f getColor() const {
        return color;
    }

    Vector3f color;          // 颜色
    Vector3f specularColor;  // 镜面反射系数
    Vector3f emission;       // 发光系数
    float shininess;         // 高光指数
    float refr;              // 折射率
    Vector3f type;           // 种类
    float relu(float x) { return std::max((float)0, x); }
};

#endif  // MATERIAL_H
