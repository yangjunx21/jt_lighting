#ifndef CURVE_HPP
#define CURVE_HPP

#include "object3d.hpp"
#include <vecmath.h>
#include <vector>
#include <utility>

#include <algorithm>

// TODO (PA2): Implement Bernstein class to compute spline basis function.
//       You may refer to the python-script for implementation.

// The CurvePoint object stores information about a point on a curve
// after it has been tesselated: the vertex (V) and the tangent (T)
// It is the responsiblility of functions that create these objects to fill in all the data.
struct CurvePoint {
    Vector3f V; // Vertex
    Vector3f T; // Tangent  (unit)
};

class Curve : public Object3D {
protected:
    std::vector<Vector3f> controls;
public:
    explicit Curve(std::vector<Vector3f> points) : controls(std::move(points)) {}

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        return false;
    }

    std::vector<Vector3f> &getControls() {
        return controls;
    }

    virtual void discretize(int resolution, std::vector<CurvePoint>& data) = 0;
};

class BezierCurve : public Curve {
public:
    explicit BezierCurve(const std::vector<Vector3f> &points) : Curve(points) {
        if (points.size() < 4 || points.size() % 3 != 1) {
            printf("Number of control points of BezierCurve must be 3n+1!\n");
            exit(0);
        }
    }

    //Bezier 曲线的递归计算，de Casteljau算法
    Vector3f calculate_P(int i, int k, float t, std::vector<Vector3f>& data){
        if(k == 0) return data[i];
        return (1 - t) * calculate_P(i, k-1, t, data) + t * calculate_P(i+1, k-1, t, data);
    }

    void discretize(int resolution, std::vector<CurvePoint>& data) override {
        data.clear();
        // TODO (PA2): fill in data vector

        //切分数量
        Vector3f direction = 1 / ( resolution * this->controls.size() + 1);
        std::vector<Vector3f> delta;   //Pi+1 - Pi，方便计算导数
        for(int i = 0 ; i < this->controls.size() - 1 ; i ++){
            delta.push_back(this->controls[i+1] - this->controls[i]);
        }

        //对于每一个t值。依次去做计算
        for(int i = 0 ; i < resolution * this->controls.size() ; i ++){
            CurvePoint p;
            float t = (float) i / (resolution * this->controls.size() + 1);
            p.V = calculate_P(0, this->controls.size() - 1, t, this->controls);
            p.T = (this->controls.size() - 1) * calculate_P(0, this->controls.size() - 2, t, delta);
            data.push_back(p);
        }
        
    }

protected:
};

class BsplineCurve : public Curve {
public:
    BsplineCurve(const std::vector<Vector3f> &points) : Curve(points) {
        if (points.size() < 4) {
            printf("Number of control points of BspineCurve must be more than 4!\n");
            exit(0);
        }
    }
    
    float calculate_B(int i, int k, float t, std::vector<float> ti_s){
        if(k == 0){
            //t_i < t < t_{i+1}，则返回1，否则返回0
            if(t < ti_s[i+1] && t >= ti_s[i]) {
                this->memory[i][k] = 1;
                return this->memory[i][k];
            }
            else{
                this->memory[i][k] = 0;
                return this->memory[i][k];
            }
        }
        else{
            if(this->memory[i][k] != -1) return this->memory[i][k];
            else{
                //递归计算
                this->memory[i][k] = (t - ti_s[i]) / (ti_s[i+k] - ti_s[i]) * calculate_B(i, k-1, t, ti_s)
                + (ti_s[i+k+1] - t) / (ti_s[i+k+1] - ti_s[i+1]) * calculate_B(i+1, k-1, t, ti_s);

                return this->memory[i][k];
            }
        }
    }

    void discretize(int resolution, std::vector<CurvePoint>& data) override {
        data.clear();
        // TODO (PA2): fill in data vector
        
        //阶数：控制点个数 - 1
        int order = this->controls.size() - 1;
        //节点向量
        std::vector<float> ti_s;
        //生成节点向量，根据规则 t_i = i / (order + k + 1)
        for(int i = 0 ; i <= order + 4 ; i ++){
            ti_s.push_back((float)i/(order+4));
        }

        for(int i = resolution * 3 ; i < resolution * (1 + order); i ++){
            CurvePoint p;
            float t = (float) i / (resolution * (order + 4));
            //重置记忆矩阵
            this->memory = new float* [order+4];
            for(int j = 0 ; j < order + 4 ; j ++){
                this->memory[j] = new float[4];
                for(int k = 0 ; k < 4 ; k ++){
                    this->memory[j][k] = -1;
                }
            }
            p.V = Vector3f::ZERO;
            p.T = Vector3f::ZERO;
            for(int j = 0 ; j < order + 1 ; j ++){
                //sum(Pi*Bi)
                p.V += calculate_B(j, 3, t, ti_s) * this->controls[j];   
                //根据导数公式计算导数
                p.T += 3 * (calculate_B(j, 2, t, ti_s) / (ti_s[j+3] - ti_s[j]) 
                    - calculate_B(j+1, 2, t, ti_s) / (ti_s[j+4] - ti_s[j+1])) * this->controls[j];
            }
            data.push_back(p);
            this->memory = NULL;
        }
    }

protected:
    //用来记录已经计算过的B，减少重复计算
    float ** memory;
};

#endif // CURVE_HPP
