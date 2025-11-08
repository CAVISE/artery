#include "Meshes.h"

#include <omnetpp.h>

#include <array>
#include <fstream>
#include <string>
#include <tuple>

void artery::sionna::meshes::createCubeObject(const std::array<double, 3>& sizes, const std::string& path) {
    // Calculate center of cube, then write 8 vertices.
    const auto [hx, hy, hz] = std::make_tuple(sizes[0] * 0.5, sizes[1] * 0.5, sizes[2] * 0.5);

    if (std::ofstream out(path); !out) {
        throw omnetpp::cRuntimeError("Cannot open file for writing: %s", path.c_str());
    } else {
        out << "# auto-generated cube OBJ\n"
            << "o Cuboid\n";

        // vertices
        out << "v " << -hx << ' ' << -hy << ' ' << -hz << '\n'
            << "v " << hx << ' ' << -hy << ' ' << -hz << '\n'
            << "v " << hx << ' ' << hy << ' ' << -hz << '\n'
            << "v " << -hx << ' ' << hy << ' ' << -hz << '\n'
            << "v " << -hx << ' ' << -hy << ' ' << hz << '\n'
            << "v " << hx << ' ' << -hy << ' ' << hz << '\n'
            << "v " << hx << ' ' << hy << ' ' << hz << '\n'
            << "v " << -hx << ' ' << hy << ' ' << hz << '\n';

        // faces
        out << "f 1 2 3 4\n"   // bottom
            << "f 5 8 7 6\n"   // top
            << "f 1 5 6 2\n"   // front
            << "f 2 6 7 3\n"   // right
            << "f 3 7 8 4\n"   // back
            << "f 5 1 4 8\n";  // left
    }
}
