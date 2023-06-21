#ifndef SPHERE_H
#define SPHERE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TODO: Implement functions and add more fields as necessary

class Sphere : public Object3D
{
public:
    Sphere()
    {
        // unit ball at the center
    }

    Sphere(const Vector3f &center, float radius, Material *material) : Object3D(material)
    {
        // ball center at 'center', with radius
        this->center = center;
        this->radius = radius;
    }

    ~Sphere() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override
    {
        //计算球心到相机的距离l
        Vector3f center_origin = center - r.getOrigin();
        //计算球心到相机的距离l再光线方向上的投影tp
        float tp = Vector3f::dot(center_origin, r.getDirection().normalized());
        //计算球心到光线方向的距离d
        float her_dis = center_origin.squaredLength() - tp * tp;
        //无交点
        if (her_dis > radius * radius)
            return false;
        //光线反方向相交
        if (tp < 0 && center_origin.squaredLength() > radius * radius)
            return false;
        float t;
        float t_ = radius * radius - her_dis;
        //球体上
        if (center_origin.squaredLength() == radius * radius)
            t = 0;
        //球体内
        else if (center_origin.squaredLength() < radius * radius)
        {
            t = tp + sqrt(t_);
        }
        //球体外
        else
        {
            t = tp - sqrt(t_);
        }
        Vector3f normal = (r.getDirection() * t + r.getOrigin() - center).normalized();
        if (t < h.getT() && t > tmin)
        {
            h.set(t, material, normal);
            return true;
        }
        else
            return false;
    }

protected:
    Vector3f center = Vector3f(0, 0, 0);
    float radius = 1.0;
};

#endif
