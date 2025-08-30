#pragma once

#include <string>
#include <vector>

namespace artery {

    namespace sionna {

        // Mitsuba provides rich interface to loading
        // and processing meshes. Sionna itself uses xml config
        // to load meshes (scene objects, generally) and material in
        // similar way INET does. However, these bridge modules provide
        // necessary abstraction to generate INET meshes in mitsuba dynamically
        // via calls to python code - much more agile approach than translating configs.
        namespace meshes {

            // Creates a temporary OBJ mesh file representing a cuboid with given sizes.
            // sizes = {X, Y, Z} in meters.
            void createCubeObject(const std::array<double, 3>& sizes, const std::string& path);

            // Writes a UV-sphere OBJ centered at origin with the given radius (meters).
            // slices: number of longitudinal segments (>=3)
            // stacks: number of latitudinal segments (>=2)
            void createSphereObject(double radius, const std::string& path, int slices = 32, int stacks = 16);

            // Extrude a 2D base polygon by height along Z.
            void createPrismObject(const std::vector<std::array<double, 2>>& base_xy, double height, const std::string& path, bool center_z = true);

            // Constructs triangles from point cloud and writes resulting polyhedron.
            void createPolyhedronObject(const std::vector<std::array<double, 3>>& points, const std::string& path);

        }  // namespace meshes

    }  // namespace sionna

}  // namespace artery