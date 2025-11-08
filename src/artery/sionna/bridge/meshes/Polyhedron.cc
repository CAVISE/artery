#include "Meshes.h"

#include <omnetpp.h>

#include <Eigen/Dense>
#include <algorithm>
#include <array>
#include <fstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using E3 = Eigen::Vector3d;

namespace {

    struct Face {
        E3 n;
        int a;
        int b;
        int c;
    };

    // Signed distance from p to plane of f (positive in direction of normal)
    double signedDistance(const std::vector<E3>& P, const Face& f, const E3& p) {
        const E3& A = P[f.a];
        return f.n.dot(p) - f.n.dot(A);
    }

    // Initial non-degenerate tetrahedron (i0, i1, i2, i3)
    std::tuple<int, int, int, int> initialTetra(const std::vector<E3>& P) {
        const std::size_t n = P.size();
        if (n < 4) {
            throw omnetpp::cRuntimeError("More than 3 points required, got %d", n);
        }

        int i0 = 0;
        // farthest from i0
        int i1 = 1;
        double best = 0.0;

        for (std::size_t i = 1; i < n; ++i) {
            double d = (P[i] - P[i0]).norm();
            if (d > best) {
                best = d;
                i1 = i;
            }
        }

        if (best == 0.0) {
            throw omnetpp::cRuntimeError("All points identical");
        }

        const E3 u = P[i1] - P[i0];
        // farthest from line i0-i1
        int i2 = -1;
        best = 0.0;

        for (std::size_t i = 0; i < n; ++i) {
            if (i != i0 && i != i1) {
                const E3 w = P[i] - P[i0];
                double area2 = u.cross(w).norm();  // proportional to distance to line * |u|
                if (area2 > best) {
                    best = area2;
                    i2 = i;
                }
            }
        }

        if (i2 < 0 || best == 0.0) {
            throw omnetpp::cRuntimeError("All points collinear");
        }

        // farthest from plane (i0, i1, i2)
        const E3 nrm = (P[i1] - P[i0]).cross(P[i2] - P[i0]);
        int i3 = -1;
        best = 0.0;

        for (std::size_t i = 0; i < n; ++i) {
            if (i != i0 && i != i1 && i != i2) {
                double vol6 = std::abs(nrm.dot(P[i] - P[i0]));
                if (vol6 > best) {
                    best = vol6;
                    i3 = i;
                }
            }
        }

        if (i3 < 0 || best == 0.0) {
            throw omnetpp::cRuntimeError("All points coplanar");
        }

        return {i0, i1, i2, i3};
    }

    // Make oriented face so normal points away from pref (pref ~ interior point)
    Face makeFaceOriented(const std::vector<E3>& P, int a, int b, int c, const E3& pref) {
        E3 n = (P[b] - P[a]).cross(P[c] - P[a]);
        if (n.dot(pref - P[a]) > 0.0) {
            std::swap(b, c);
            n = -n;
        }

        double ln = n.norm();
        if (ln > 0.0) {
            n /= ln;
        }

        return {
            .n = n,
            .a = a,
            .b = b,
            .c = c,
        };
    }

    void addPoint2Hull(const std::vector<E3>& P, int ip, std::vector<Face>& faces, const double eps = 1e-9) {
        std::vector<char> vis(faces.size(), 0);
        for (std::size_t i = 0; i < faces.size(); ++i) {
            if (signedDistance(P, faces[i], P[ip]) > eps) {
                vis[i] = 1;
            }
        }

        std::vector<std::pair<std::pair<int, int>, int>> edgemap;
        struct Edge {
            int u;
            int v;
        };

        auto canon = [](int a, int b) { return std::make_pair(std::min(a, b), std::max(a, b)); };

        auto addEdge = [&](int u, int v) {
            auto key = canon(u, v);
            auto it = std::find_if(edgemap.begin(), edgemap.end(), [&](auto& kv) { return kv.first == key; });
            if (it == edgemap.end()) {
                edgemap.push_back({key, 1});
            } else {
                it->second++;
            }
        };

        for (std::size_t i = 0; i < faces.size(); ++i) {
            if (vis[i]) {
                auto& f = faces[i];
                addEdge(f.a, f.b);
                addEdge(f.b, f.c);
                addEdge(f.c, f.a);
            }
        }

        std::vector<Edge> horizon;
        for (auto& kv : edgemap) {
            if (kv.second == 1) {
                horizon.push_back({kv.first.first, kv.first.second});
            }
        }

        std::vector<Face> kept;
        kept.reserve(faces.size());
        for (std::size_t i = 0; i < faces.size(); ++i) {
            if (!vis[i]) {
                kept.push_back(faces[i]);
            }
        }

        E3 centroid = E3::Zero();
        for (auto& f : kept) {
            centroid += (P[f.a] + P[f.b] + P[f.c]) / 3.0;
        }

        if (!kept.empty()) {
            centroid /= static_cast<double>(kept.size());
        }

        for (auto e : horizon) {
            kept.push_back(makeFaceOriented(P, e.u, e.v, ip, centroid));
        }

        faces.swap(kept);
    }

    std::vector<Face> convexHull3D(const std::vector<E3>& P, const std::size_t approxFaces = 64, const double eps = 1e-9) {
        auto [i0, i1, i2, i3] = initialTetra(P);

        // initial hull (tetra), oriented outward
        E3 C = (P[i0] + P[i1] + P[i2] + P[i3]) / 4.0;
        std::vector<Face> faces;
        faces.reserve(approxFaces);

        faces.push_back(makeFaceOriented(P, i0, i1, i2, C));
        faces.push_back(makeFaceOriented(P, i0, i3, i1, C));
        faces.push_back(makeFaceOriented(P, i1, i3, i2, C));
        faces.push_back(makeFaceOriented(P, i2, i3, i0, C));

        const std::size_t n = P.size();

        for (std::size_t i = 0; i < n; ++i) {
            if (i == i0 || i == i1 || i == i2 || i == i3) {
                continue;
            }

            bool outside = false;
            for (auto& f : faces)
                if (signedDistance(P, f, P[i]) > eps) {
                    outside = true;
                    break;
                }

            if (outside) {
                addPoint2Hull(P, i, faces);
            }
        }

        return faces;
    }

}  // namespace

void artery::sionna::meshes::createPolyhedronObject(const std::vector<std::array<double, 3>>& points, const std::string& path) {
    std::vector<E3> P;

    if (const std::size_t n = points.size(); n < 4) {
        throw omnetpp::cRuntimeError("More than 3 points required, got %d", n);
    } else {
        P.reserve(n);
        for (auto& q : points) {
            P.emplace_back(q[0], q[1], q[2]);
        }
    }

    auto faces = convexHull3D(P);

    if (std::ofstream out(path); !out) {
        throw omnetpp::cRuntimeError("Cannot open file for writing: %s", path.c_str());
    } else {
        out << "# auto-generated polyhedron OBJ (convex hull, Eigen)\n";
        out << "o Polyhedron\n";

        for (auto& v : P) {
            out << "v " << v.x() << ' ' << v.y() << ' ' << v.z() << '\n';
        }

        for (auto& f : faces) {
            out << "f " << (f.a + 1) << ' ' << (f.b + 1) << ' ' << (f.c + 1) << '\n';
        }
    }
}
