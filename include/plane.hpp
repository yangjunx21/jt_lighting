#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TODO: Implement Plane representing an infinite plane
// function: ax+by+cz=d
// choose your representation , add more fields and fill in the functions

class Plane : public Object3D {
public:
    Plane() {
        //should not be used
    }

    Plane(const Vector3f &normal, float d, Material *m) : Object3D(m) {
        this->norm = normal.normalized();
        this->dis = d;
    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        float t = - (norm.dot(norm, r.getOrigin())-dis) / norm.dot(norm, r.getDirection());
        //判断t是否是当前最近的交点，并且需要保证交点的t值大于tmin
        if( t < h.getT() && t > tmin ){
            h.set(t, material, norm);
            return true;
        }
        else 
            return false;
    }

protected:
    Vector3f norm = Vector3f(0, 0, 0);
    float dis = 0;
};

#endif //PLANE_H
		

