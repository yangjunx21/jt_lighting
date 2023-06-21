#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>
using namespace std;

// TODO: implement this class and add more fields as necessary,
class Triangle : public Object3D
{

public:
	Triangle() = delete;

	// a b c are three vertex positions of the triangle
	Triangle(const Vector3f &a, const Vector3f &b, const Vector3f &c, Material *m) : Object3D(m)
	{
		vertices[0] = a;
		vertices[1] = b;
		vertices[2] = c;
		normal = vertices[0].cross(vertices[0]-vertices[1], vertices[0]-vertices[2]).normalized();
		d = normal.dot(normal, vertices[0]);
	}

	//用 inside 函数判断点 P 是否在三角形的偏内侧一边
	bool inside(Vector3f a, Vector3f b, Vector3f c, Vector3f d){
		Vector3f e1 = b - a;
		Vector3f e2 = c - a;
		Vector3f e3 = d - a;
		
		//计算两个叉乘是否方向相同
		Vector3f p1 = Vector3f::cross(e1, e2);
		Vector3f p2 = Vector3f::cross(e1, e3);
		return Vector3f::dot(p1, p2) >= 0;
	}

	bool intersect(const Ray &ray, Hit &hit, float tmin) override
	{
		float t = - (normal.dot(normal, ray.getOrigin())-d) / normal.dot(normal, ray.getDirection());
		Vector3f P = ray.getOrigin() + t * ray.getDirection();
		//是否在三角形内
		bool flag = inside(vertices[0], vertices[1], vertices[2], P)
		&& inside(vertices[1], vertices[2], vertices[0], P)
		&& inside(vertices[2], vertices[0], vertices[1], P);
		if (flag && t < hit.getT() && t > tmin)
		{
			hit.set(t, material, normal);
			return true;
		}
		else
			return false;
	}

	Vector3f normal;
	Vector3f vertices[3];
protected:
	float d;
};

#endif // TRIANGLE_H
