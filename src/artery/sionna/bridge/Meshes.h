#pragma once

#include <vector>
#include <string>
#include <ostream>

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Helpers.h>
#include <drjit/array.h>
#include <drjit/array_router.h>

/**
 * @file Mitsuba provides rich interface to loading
 * and processing meshes. Sionna itself uses xml config
 * to load meshes (scene objects, generally) and material in
 * similar way INET does. However, these bridge modules provide
 * necessary abstraction to generate INET meshes in mitsuba dynamically
 * via calls to python code - much more agile approach than translating configs.
 */

namespace artery {

    namespace sionna {

        MI_VARIANT
        class Meshes {
        public:
            
            SIONNA_IMPORT_CORE_TYPES(Float64, Float32, Int32, Vector3d, Vector2d)

            Meshes() = delete;

            /**
             * @brief Creates a temporary OBJ mesh file representing a cuboid with given sizes.
             * 
             * This works by finding center of a cube, and then writing vertices relevant
             * to that center.
             *
             * @param sizes {X, Y, Z} in meters.
             * @param path OBJ file to write to.
             */
            static void createCubeObject(std::ostream& os, const Vector3d& sizes);

            /**
             * @brief Writes a UV-sphere OBJ centered at origin with the given radius (meters).
             * 
             * @param slices slices: number of longitudinal segments (>=3)
             * @param stacks stacks: number of latitudinal segments (>=2)
             */
            static void createSphereObject(std::ostream& os, Float64 radius, Int32 slices = 32, Int32 stacks = 16);

            // Extrsude a 2D base polygon by height along Z.
            static void createPrismObject(std::ostream& os, const std::vector<Vector2d>& base_xy, double height, bool center_z = true);

            // Constructs triangles from point cloud and writes resulting polyhedron.
            static void createPolyhedronObject(std::ostream& os, const std::vector<Vector3d>& points);

        };

    }  // namespace sionna

}  // namespace artery

MI_VARIANT
void artery::sionna::Meshes<Float, Spectrum>::createCubeObject(std::ostream& os, const Vector3d& sizes) {
    // Calculate center of cube, then write 8 vertices.
    Vector3d halfs = sizes * Float32(0.5);

    Float64 hx = halfs[0];
    Float64 hy = halfs[1];
    Float64 hz = halfs[2];

    std::ios_base::iostate state = os.exceptions();
    os.exceptions(std::ostream::badbit | std::ostream::failbit);

    try {
        os  << "# auto-generated cube OBJ\n"
            << "o Cuboid\n";
    
        // vertices
        os  << "v " << -hx << ' ' << -hy << ' ' << -hz << '\n'
            << "v " << hx << ' ' << -hy << ' ' << -hz << '\n'
            << "v " << hx << ' ' << hy << ' ' << -hz << '\n'
            << "v " << -hx << ' ' << hy << ' ' << -hz << '\n'
            << "v " << -hx << ' ' << -hy << ' ' << hz << '\n'
            << "v " << hx << ' ' << -hy << ' ' << hz << '\n'
            << "v " << hx << ' ' << hy << ' ' << hz << '\n'
            << "v " << -hx << ' ' << hy << ' ' << hz << '\n';
    
        // faces
        os  << "f 1 2 3 4\n"   // bottom
            << "f 5 8 7 6\n"   // top
            << "f 1 5 6 2\n"   // front
            << "f 2 6 7 3\n"   // right
            << "f 3 7 8 4\n"   // back
            << "f 5 1 4 8\n";  // left
    } catch (const std::ios_base::failure& err) {
        throw sionna::wrapRuntimeError("output stream failure: %s", err.what());
    }

    os.exceptions(state);
}

MI_VARIANT
void artery::sionna::Meshes<Float, Spectrum>::createSphereObject(std::ostream& os, Float64 radius, Int32 slices, Int32 stacks) {
    // NOOP.
}

MI_VARIANT
void artery::sionna::Meshes<Float, Spectrum>::createPrismObject(std::ostream& os, const std::vector<Vector2d>& base_xy, double height, bool center_z) {
    // NOOP.
}

MI_VARIANT
void artery::sionna::Meshes<Float, Spectrum>::createPolyhedronObject(std::ostream& os, const std::vector<Vector3d>& points) {
    // NOOP.
}
