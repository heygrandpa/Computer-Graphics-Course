/***************************************************************************
* basic_rasterizer.cpp    (rasterizer plugin)                              *
*                                                                          *
* A "rasterizer" is responsible for tracing all the primary rays needed to *
* create an image, filling in the resulting matrix of color values (i.e.   *
* the "raster"), and saving the results as an image file.  This "basic"    *
* rasterizer simply casts a single ray per pixel and saves the result as   *
* a PPM image.  It does no anti-aliasing.                                  *
*                                                                          *
* History:                                                                 *
*   10/03/2005  Made rasterizer a plugin.  Line numbers written in place.  *
*   12/19/2004  Initial coding.                                            *
*                                                                          *
***************************************************************************/
#include "toytracer.h"
#include "ppm_image.h"
#include "params.h"
#include "util.h"

struct basic_rasterizer : public Rasterizer {
    basic_rasterizer() { }

    virtual ~basic_rasterizer() { }

    virtual bool Rasterize(string fname, const Camera &, const Scene &) const;

    virtual Plugin *ReadString(const string &params);

    virtual string MyName() const { return "basic_rasterizer"; }

    virtual bool Default() const { return true; }
};

REGISTER_PLUGIN(basic_rasterizer);

Plugin *basic_rasterizer::ReadString(const string &params) {
    ParamReader p(params);
    if (p["rasterizer"] && p[MyName()]) return new basic_rasterizer();
    return NULL;
}

// This is a trivial tone mapper; it merely maps values that are
// in [0,1] and maps them to integers between 0 and 255.  If the
// real value is above 1, it merely truncates.  A true tone mapper
// would attempt to handle very large values nicely, without
// truncation; that is, it would try to compensate for the fact that
// displays have a very limited dynamic range.
static Pixel ToneMap(const Color &color) {
    int red = (int) floor(256 * color.red);
    int green = (int) floor(256 * color.green);
    int blue = (int) floor(256 * color.blue);
    channel r = (channel) (red >= 255 ? 255 : red);
    channel g = (channel) (green >= 255 ? 255 : green);
    channel b = (channel) (blue >= 255 ? 255 : blue);
    return Pixel(r, g, b);
}

// Rasterize casts all the initial rays starting from the eye.
// This trivial version simply casts one ray per pixel, in raster
// order, then writes the pixels out to a file.
bool basic_rasterizer::Rasterize(string file_name, const Camera &cam, const Scene &scene) const {
    file_name += ".ppm";

    // Make sure the file is accessible by overwriting it now.  That way, if the file
    // is not accessible, we'll find out now instead of waiting until the image is ready
    // to be written.

    if (!Overwrite_PPM_Image(file_name)) {
        cerr << "Error: Could not open file " << file_name << " for writing." << endl;
        return false;
    }

    // Create an image of the given resolution.

    PPM_Image I(cam.x_res, cam.y_res);

    // Initialize all the fields of the first-generation ray except for "direction".

    Ray ray;
    ray.origin = cam.eye;     // All initial rays originate from the eye.
    ray.type = generic_ray; // These rays are given no special meaning.
    ray.generation = 1;           // Rays cast from the eye are first-generation.

    const double xmin = cam.x_win.min;
    const double ymax = cam.y_win.max;
    const double width = Len(cam.x_win);
    const double height = Len(cam.y_win);

    // Compute increments etc. based on the camera geometry.  These will be used
    // to define the ray direction at each pixel.

    const Vec3 G(Unit(cam.lookat - cam.eye));          // Gaze direction.
    const Vec3 U(Unit(cam.up / G));                    // Up vector.
    const Vec3 R(Unit(G ^ U));                         // Right vector.
    const Vec3 O(cam.vpdist * G + xmin * R + ymax * U);  // "Origin" of the 3D raster.
    const Vec3 dR(width * R / cam.x_res);                // Right increments.
    const Vec3 dU(height * U / cam.y_res);                // Up increments.

    // Loop over the entire image, casting a single ray per pixel.  This nested loop
    // must be modified to accommodate anti-aliasing.

    cout << "Rendering line 0";
    for (unsigned int i = 0; i < cam.y_res; i++) {
        // Overwrite the line number written to the console.
        cout << rubout(i) << (i + 1);
        cout.flush();
        for (unsigned int j = 0; j < cam.x_res; j++) {
            Color sum;
            for (int tx = -1; tx < 2; ++tx) {
                for (int ty = -1; ty < 2; ++ty) {
                    ray.direction = Unit(O + (j + ty / 4.0 + 0.5) * dR - (i + tx / 4.0 + 0.5) * dU);
                    sum += scene.Trace(ray);
                }
            }
            I(i, j) = ToneMap(sum / 9);
        }
    }

    // Thus far the image exists only in memory.  Now write it out to a file.

    cout << "\nWriting image file " << file_name << "... ";
    cout.flush();
    I.Write(file_name);
    cout << "done." << endl;
    return true;
}
