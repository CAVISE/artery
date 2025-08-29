#include "Meshes.h"

#include <omnetpp.h>

#include <cmath>
#include <fstream>
#include <string>

using namespace artery::sionna;

void meshes::createSphereObject(double radius, const std::string& path, int slices, int stacks) {
    if (radius <= 0.0) {
        throw omnetpp::cRuntimeError("radius must be greater than 0, but is %d", radius);
    } else if (slices < 3) {
        throw omnetpp::cRuntimeError("slices count must be greater than 3, but is %d", slices);
    } else if (stacks < 2) {
        throw omnetpp::cRuntimeError("stacks count must be greater than 2, but is %d", stacks);
    }

    if (std::ofstream out(path); !out) {
        throw omnetpp::cRuntimeError("Cannot open file for writing: %s", path.c_str());
    } else {
        out << "# auto-generated sphere OBJ\n";
        out << "o Sphere\n";

        const int cols = slices + 1;
        auto idx = [&cols](int s, int l) { return s * cols + l; };

        for (int s = 0; s <= stacks; ++s) {
            double v = static_cast<double>(s) / stacks;
            double theta = v * M_PI;
            double sinT = std::sin(theta);
            double cosT = std::cos(theta);

            for (int l = 0; l <= slices; ++l) {
                double u = static_cast<double>(l) / slices;
                double phi = u * 2.0 * M_PI;
                double sinP = std::sin(phi);
                double cosP = std::cos(phi);

                double nx = cosP * sinT;
                double ny = sinP * sinT;
                double nz = cosT;

                double x = radius * nx;
                double y = radius * ny;
                double z = radius * nz;

                out << "v " << x << ' ' << y << ' ' << z << '\n';
                out << "vn " << nx << ' ' << ny << ' ' << nz << '\n';
            }
        }

        // Faces: two triangles per quad (v00,v10,v11) and (v00,v11,v01)
        auto write_tri = [&out](int a, int b, int c) {
            int A = a + 1, B = b + 1, C = c + 1;
            out << "f " << A << "//" << A << ' ' << B << "//" << B << ' ' << C << "//" << C << '\n';
        };

        for (int s = 0; s < stacks; ++s) {
            for (int l = 0; l < slices; ++l) {
                int v00 = idx(s, l);
                int v01 = idx(s, l + 1);
                int v10 = idx(s + 1, l);
                int v11 = idx(s + 1, l + 1);

                // avoiding degenerate triangles at the poles is optional;
                // this standard pattern works fine across stacks.
                write_tri(v00, v10, v11);
                write_tri(v00, v11, v01);
            }
        }
    }
}
