#include "Meshes.h"

#include <omnetpp.h>

#include <algorithm>
#include <array>
#include <fstream>
#include <string>
#include <vector>

using namespace artery::sionna;

namespace {

    // Returns squared area for arbitrary polyhedra.
    double signedArea2(const std::vector<std::array<double, 2>>& poly) {
        double a = 0.0;
        for (std::size_t i = 0, n = poly.size(); i < n; ++i) {
            const auto& p = poly[i];
            const auto& q = poly[(i + 1) % n];
            a += p[0] * q[1] - q[0] * p[1];
        }

        return a;
    }

    bool pointInTri(double ax, double ay, double bx, double by, double cx, double cy, double px, double py) {
        auto sgn = [](double x1, double y1, double x2, double y2, double x3, double y3) { return (x1 - x3) * (y2 - y3) - (x2 - x3) * (y1 - y3); };

        double d1 = sgn(px, py, ax, ay, bx, by);
        double d2 = sgn(px, py, bx, by, cx, cy);
        double d3 = sgn(px, py, cx, cy, ax, ay);

        bool hasNegative = (d1 < 0) || (d2 < 0) || (d3 < 0);
        bool hasPositive = (d1 > 0) || (d2 > 0) || (d3 > 0);
        return !(hasNegative && hasPositive);
    }

    bool isEar(const std::vector<std::array<double, 2>>& poly, const std::vector<int>& idxs, int i, const double eps = 1e-12) {
        const int n = static_cast<int>(idxs.size());

        int i0 = idxs[(i - 1 + n) % n];
        int i1 = idxs[i];
        int i2 = idxs[(i + 1) % n];

        const auto& A = poly[i0];
        const auto& B = poly[i1];
        const auto& C = poly[i2];

        double cross = (B[0] - A[0]) * (C[1] - A[1]) - (B[1] - A[1]) * (C[0] - A[0]);
        if (cross <= eps) {
            return false;
        }

        for (int j = 0; j < n; ++j) {
            if (int k = idxs[j]; k == i0 || k == i1 || k == i2) {
                continue;
            } else {
                const auto& P = poly[k];
                if (pointInTri(A[0], A[1], B[0], B[1], C[0], C[1], P[0], P[1])) {
                    return false;
                }
            }
        }

        return true;
    }

    // this cuts provided shape to remove ears: https://en.wikipedia.org/wiki/Polygon_triangulation
    std::vector<std::array<int, 3>> triangulateCCW(const std::vector<std::array<double, 2>>& polyCCW, const std::size_t maxIters = 10000) {
        std::vector<int> idxs;
        std::vector<std::array<int, 3>> tris;

        if (const std::size_t n = polyCCW.size(); n < 3) {
            return tris;
        } else {
            idxs.resize(n);
            for (std::size_t i = 0; i < n; ++i) {
                idxs[i] = i;
            }
        }

        int guard = 0;
        while (idxs.size() > 3 && guard < maxIters) {
            bool cut = false;
            const std::size_t n = idxs.size();

            for (std::size_t i = 0; i < n; ++i) {
                if (isEar(polyCCW, idxs, i)) {
                    int i0 = idxs[(i - 1 + n) % n];
                    int i1 = idxs[i];
                    int i2 = idxs[(i + 1) % n];

                    tris.push_back({i0, i1, i2});
                    idxs.erase(idxs.begin() + i);
                    cut = true;
                    break;
                }
            }

            if (!cut) {
                // fallback: fan triangulation from vertex 0
                tris.clear();
                for (std::size_t i = 1; i < polyCCW.size() - 1; ++i) {
                    int i0 = 0;
                    int i1 = i;
                    int i2 = i + 1;

                    tris.push_back({i0, i1, i2});
                }
                return tris;
            }

            ++guard;
        }

        if (idxs.size() == 3) {
            tris.push_back({idxs[0], idxs[1], idxs[2]});
        }

        return tris;
    }
}  // namespace


void meshes::createPrismObject(const std::vector<std::array<double, 2>>& base_xy, double height, const std::string& path, bool center_z) {
    if (height <= 0.0) {
        throw omnetpp::cRuntimeError("height must be greater than 0, but is %d", height);
    } else if (std::size_t points = base_xy.size(); points < 3) {
        throw omnetpp::cRuntimeError("base polygon needs at least 3 points, but their count is %d", points);
    }

    std::vector<std::array<double, 2>> poly = base_xy;
    if (signedArea2(poly) <= 0.0) {
        std::reverse(poly.begin(), poly.end());
    }

    const std::size_t n = poly.size();
    auto cap_tris = triangulateCCW(poly);

    const double z0 = center_z ? -0.5 * height : 0.0;
    const double z1 = center_z ? 0.5 * height : height;

    if (std::ofstream out(path); !out) {
        throw omnetpp::cRuntimeError("Cannot open file for writing: %s", path.c_str());
    } else {
        out << "# auto-generated prism OBJ\n";
        out << "o Prism\n";

        // Vertices: base ring at z0, then top ring at z1
        for (std::size_t i = 0; i < n; ++i) {
            out << "v " << poly[i][0] << ' ' << poly[i][1] << ' ' << z0 << '\n';
        }
        for (std::size_t i = 0; i < n; ++i) {
            out << "v " << poly[i][0] << ' ' << poly[i][1] << ' ' << z1 << '\n';
        }

        // Faces: triangles only
        auto emit_tri = [&out](int a, int b, int c) { out << "f " << (a + 1) << " " << (b + 1) << " " << (c + 1) << "\n"; };

        // Base cap (normals should point outward = downwards on base): cap_tris are CCW seen from +Z.
        for (auto t : cap_tris) {
            emit_tri(t[2], t[1], t[0]);
        }

        // Top cap (outward = +Z): same CCW order but offset by +n
        for (auto t : cap_tris) {
            emit_tri(t[0] + n, t[1] + n, t[2] + n);
        }

        // Side faces: each quad (i -> i+1) becomes 2 triangles
        for (std::size_t i = 0; i < n; ++i) {
            int i0 = i;
            int i1 = (i + 1) % n;
            int j0 = i0 + n;  // top ring
            int j1 = i1 + n;
            // Winding chosen for outward normals
            emit_tri(i0, i1, j1);
            emit_tri(i0, j1, j0);
        }
    }
}
