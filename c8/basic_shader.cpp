/***************************************************************************
* basic_shader.cpp   (shader plugin)                                       *
*                                                                          *
* This file defines a very simple ray tracing shader for the toy tracer.   *
* The job of the shader is to determine the color of the surface, viewed   *
* from the origin of the ray that hit the surface, taking into account the *
* surface material, light sources, and other objects in the scene.         *                          *
*                                                                          *
* History:                                                                 *
*   10/03/2005  Updated for Fall 2005 class.                               *
*   09/29/2004  Updated for Fall 2004 class.                               *
*   04/14/2003  Point lights are now point objects with emission.          *
*   04/01/2003  Initial coding.                                            *
*                                                                          *
***************************************************************************/
#include "toytracer.h"
#include "util.h"
#include "params.h"

struct basic_shader : public Shader {
    basic_shader() { }

    ~basic_shader() { }

    virtual Color Shade(const Scene &, const HitInfo &) const;

    virtual Plugin *ReadString(const string &params);

    virtual string MyName() const { return "basic_shader"; }

    virtual bool Default() const { return true; }
};

REGISTER_PLUGIN(basic_shader);

Plugin *basic_shader::ReadString(const string &params) {
    ParamReader get(params);
    if (get["shader"] && get[MyName()]) return new basic_shader();
    return NULL;
}


Color basic_shader::Shade(const Scene &scene, const HitInfo &hit) const {
    // ********* Keep as little or as much of the following code as you wish *******
    // ********* Keep as little or as much of the following code as you wish *******
    // ********* Keep as little or as much of the following code as you wish *******
    Ray ray;
    HitInfo otherhit;
    static const double epsilon = 1.0E-4;
    if (Emitter(hit.object)) return hit.object->material->emission;

    Material *mat = hit.object->material;
    Color diffuse = mat->diffuse;
    Color specular = mat->specular;
    Color color = mat->ambient * diffuse;
    Vec3 O = hit.ray.origin;
    Vec3 P = hit.point;
    Vec3 N = hit.normal;
    Vec3 E = Unit(O - P);
    Vec3 R = Unit((2.0 * (E * N)) * N - E);
    Color r = mat->reflectivity;
    double e = mat->Phong_exp;
    double k = mat->ref_index;


    if (E * N < 0.0) N = -N;  // Flip the normal if necessary.

    ray.origin = P + R * epsilon;
    ray.direction = R;
    ray.type = indirect_ray;
    ray.generation = hit.ray.generation + 1;
    color += scene.Trace(ray) * r;

    for (unsigned i = 0; i < scene.NumLights(); i++) {
        const Object *light = scene.GetLight(i);
        Color emission = light->material->emission;
        AABB box = GetBox(*light);
        Vec3 LightPos(Center(box));
        Vec3 lightDir = Unit(LightPos - P);


        ray.type = shadow_ray;
        ray.origin = P + lightDir * epsilon;
        ray.direction = lightDir;
        otherhit.distance = Infinity;

        if (scene.Cast(ray, otherhit)) {
            continue;
        }

        if (N * lightDir <= 0) continue;

        double diff = N * lightDir;
        color += diff * diffuse * emission;

        if (e == 0) continue;
        double reflect = 2.0f * (diff);
        Vec3 phongDir = lightDir - reflect * N;
        color += specular * pow(max(-phongDir * E, 0), e);


    }

    Vec3 T = -Unit(E);
    double cosT = N * E;
    if (1 - k * k * cosT * cosT > 0) {
        double sinkT = k * sqrt(1 - cosT * cosT);
        double coskT = sqrt(1 - sinkT * sinkT);
        T = k * T + ((k * cosT - coskT) * N);
        ray.origin = P + T * epsilon;
        ray.direction = T;
        ray.type = indirect_ray;
        ray.generation = hit.ray.generation + 1;
        color = color * (White - mat->translucency);
        color += scene.Trace(ray) * mat->translucency;
    }

    color.unit();
    return color;
}
