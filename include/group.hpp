#ifndef GROUP_H
#define GROUP_H


#include "object3d.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include <iostream>
#include <vector>


// TODO: Implement Group - add data structure to store a list of Object*
class Group : public Object3D {

public:

    Group() {
        //do nothing
    }

    explicit Group (int num_objects) {
        //declare the total number
        for(int i = 0 ; i < num_objects ; i ++)
            objects.push_back(nullptr);
        num = num_objects;
    }

    ~Group() override {
        delete objects.back();
        objects.pop_back();
    }

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        bool flag = false;
        for(int i  = 0 ; i < num ; i ++){
            //依次对每一个物体求交
            if(objects[i]->intersect(r, h, tmin))
                flag = true;
        }
        return flag;
    }

    void addObject(int index, Object3D *obj) {
        objects[index] = obj;
    }

    int getGroupSize() {
        return num;
    }

private:
    std::vector<Object3D*> objects;
    //物体总数
    int num;
};

#endif
	
