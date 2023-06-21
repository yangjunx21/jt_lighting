#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <random>

#include <vecmath.h>

#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"
#include "material.hpp"

#include <string>

using namespace std;

static std::mt19937 mersenneTwister;
static std::uniform_real_distribution<float> uniform;
#define RND1 (2.0 * uniform(mersenneTwister) - 1.0) //[-1, 1]均匀分布
#define RND2 (uniform(mersenneTwister))
inline float ReLU(float x)
{
    return x < 0 ? 0 : x > 1 ? 1
                             : x;
}
inline float toFloat(float x) { return float(int(pow(ReLU(x), 1 / 2.2) * 255 + .5)) / 255.0; }
inline float clamp(float x) { return x < 0 ? 0 : x > 1 ? 1
                                                       : x; }

Vector3f radiance(const Ray &r, int depth, Group *basegroup)
{
    // printf("depth = %d\n",depth);
    Hit hit;
    bool isintersect = basegroup->intersect(r, hit, 0);
    Material *material = hit.getMaterial();
    int id = 0; // id of intersected object
    if (!isintersect)
        return Vector3f::ZERO; // if miss, return black
    if (material->emission != Vector3f::ZERO)
        return material->emission;
    // if(material->emission != Vector3f::ZERO)
    //     return material->emission;

    Vector3f x = r.getDirection().normalized() * hit.getT() + r.getOrigin(), n = hit.getNormal().normalized(), nl = n.dot(r.getDirection().normalized(), n) < 0 ? n : n * (-1), f = hit.getMaterial()->getColor();
    float p = f.x() > f.y() && f.x() > f.z() ? f.x() : f.y() > f.z() ? f.y()
                                                                     : f.z(); // max refl
    //p = p < 0.75 ? p : 0.75;

    if (++depth > 5)
    {
        if (RND1 < p)
            f = f * (1 / p);
        else
            return material->emission; // R.R.
        // if(depth >= 10000)
        //     return material->emission;
    }
    Vector3f hit_emission = material->emission;
    Vector3f hit_normal = hit.getNormal().normalized();
    Vector3f next_origin = r.getOrigin() + hit.getT() * r.getDirection().normalized();
    Vector3f ray_direction = r.getDirection().normalized();

    float b = Vector3f::dot(ray_direction, hit_normal);
    if (material->type.x() == 1)
    { // 漫反射
        float r1 = 2 * 3.14159265 * RND2, r2 = RND2, r2s = sqrt(r2);
        Vector3f w = nl, u = (Vector3f::cross((fabs(w.x()) > .1 ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0)), w)).normalized(), v = Vector3f::cross(w, u);
        Vector3f d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).normalized();

        Vector3f next_color = radiance(Ray(next_origin, d), depth, basegroup);

        return hit_emission + f * next_color;
    }
    else if (material->type.y() == 1)
    { // 镜面反射
        Vector3f next_direction = ray_direction - n * (b * 2);
        next_direction.normalize();
        Vector3f next_color = radiance(Ray(next_origin, next_direction), depth, basegroup);
        return hit_emission + f * next_color;
    }
    else
    { // 折射
        Ray reflray(next_origin, ray_direction - n * (2 * b));
        bool into = n.dot(nl, n) > 0;
        float nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = ray_direction.dot(nl, ray_direction), cos2t;
        if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0) // Total internal reflection
            return material->emission + f * radiance(reflray, depth, basegroup);
        Vector3f tdir = (ray_direction * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).normalized();
        float a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c = 1 - (into ? -ddn : tdir.dot(n, tdir));
        float Re = R0 + (1 - R0) * c * c * c * c * c, Tr = 1 - Re, P = .25 + .5 * Re, RP = Re / P, TP = Tr / (1 - P);
        return material->emission + f * (depth > 2 ? (RND2 < P ? // Russian roulette
                                                          radiance(reflray, depth, basegroup) * RP
                                                               : radiance(Ray(x, tdir), depth, basegroup) * TP)
                                                   : radiance(reflray, depth, basegroup) * Re + radiance(Ray(x, tdir), depth, basegroup) * Tr);
        // float n = material->refr;
        // float R0 = ((1.0 - n) * (1.0 - n)) / ((1.0 + n) * (1.0 + n));
        // if (b > 0)
        // { //??存疑
        //     hit_normal.negate();
        // }
        // else
        // {
        //     n = 1.0 / n;
        // }

        // float cos1 = -Vector3f::dot(hit_normal, ray_direction);
        // float cos2 = 1.0 - n * n * (1.0 - cos1 * cos1); // cos(theta2)的平方
        // Vector3f reflect = (ray_direction + hit_normal * (cos1 * 2));
        // if (cos2 < 0)
        // {
        //     return hit_emission + f * radiance(Ray(next_origin, reflect), depth, basegroup);
        // }
        // // Schlick估计菲涅尔项
        // float Rprob = R0 + (1.0 - R0) * pow(1.0 - cos1, 5.0);

        // Vector3f refrac = ((ray_direction * n) + (hit_normal * (n * cos1 - sqrt(cos2)))).normalized();

        // float P = 0.25 + 0.5 * Rprob;
        // if (depth <= 1)
        //     return hit_emission + f * (Rprob * radiance(Ray(next_origin, reflect), depth + 1, basegroup) + (1 - Rprob) * radiance(Ray(next_origin, refrac), depth + 1, basegroup));

        // if (RND2 < P)
        // {
        //     return hit_emission + f * ((Rprob / P) * radiance(Ray(next_origin, reflect), depth + 1, basegroup));
        // }
        // else
        // {
        //     return hit_emission + f * (((1 - Rprob) / (1 - P)) * radiance(Ray(next_origin, refrac), depth + 1, basegroup));
        // }
    }
}

int main(int argc, char *argv[])
{
    for (int argNum = 1; argNum < argc; ++argNum)
    {
        std::cout << "Argument " << argNum << " is: " << argv[argNum] << std::endl;
    }

    if (argc != 3)
    {
        cout << "Usage: ./bin/PA1 <input scene file> <output bmp file>" << endl;
        return 1;
    }
    string inputFile = argv[1];
    string outputFile = argv[2]; // only bmp is allowed.

    // TODO: Main RayCasting Logic
    // First, parse the scene using SceneParser.
    // Then loop over each pixel in the image, shooting a ray
    // through that pixel and finding its intersection with
    // the scene.  Write the color at the intersection to that
    // pixel in your output image.
    // 循 环 屏 幕 空 间 的 像 素
    SceneParser sceneparser = SceneParser(inputFile.c_str());
    Camera *camera = sceneparser.getCamera();
    Image Img(camera->getWidth(), camera->getHeight());
    Group *baseGroup = sceneparser.getGroup();
    int samps = 20;
    int w = camera->getWidth();
    int h = camera->getHeight();
#pragma omp parallel for schedule(dynamic, 1) // OpenMP
    for (int y = 0; y < h; y++)
    { // Loop over image rows
        fprintf(stderr, "\rRendering (%d spp) %5.2f%%", samps, 100. * y / (h - 1));
        // unsigned short Xi[3] = {0, 0, (unsigned short)(y * y * y)};
        for (int x = 0; x < w; x++)
        {
            Vector3f r = Vector3f::ZERO;
            for (int sy = 0; sy < 2; sy++)
            { // 2x2 subpixel rows
                for (int sx = 0; sx < 2; sx++)
                { // 2x2 subpixel cols
                    for (int s = 0; s < samps; ++s)
                    {
                        Ray camRay = sceneparser.getCamera()->generateRay(Vector2f(x + RND1, y + RND1));
                        r = r + radiance(camRay, 0, baseGroup) * (1. / samps) / 4;
                        // r = ptColor(camRay, sceneparser);
                        // cout << r.x() << " " << r.y() << " " << r.z() << std::endl;
                    }
                }
            }
            Img.SetPixel(x, y, Vector3f(toFloat(clamp(r.x())),toFloat(clamp(r.y())), toFloat(clamp(r.z()))));
        } // Camera rays are pushed ^^^^^ forward to start in interior
    }
    Img.SaveImage(outputFile.c_str());

    cout << "Hello! Computer Graphics!" << endl;
    return 0;
}
