#include <cavise/sionna/bridge/EnvironmentMeshGenerator.h>
#include <cavise/sionna/bridge/Fwd.h>
#include <cavise/sionna/bridge/Helpers.h>
#include <cmath>

using Float = float;
using Spectrum = mitsuba::Color<Float, 3>;

namespace dr = drjit;

MI_IMPORT_CORE_TYPES()

namespace artery::sionna::meshes {

Vector3f normalized(const Vector3f& p) {
        float n = norm(p);
        if (n < 1e-9F) {
            return Vector3f(0.F);
        }
        return normalize(p);
}

// Centroid of a list of points
Vector3f centroid(const std::vector<Vector3f>& pts) {
    Vector3f c(0.f);
    for (const auto& p : pts) {
        c += p;
    }
    float n = static_cast<float>(pts.size());
    return c / n;
};

// ---------------------------------------------------------------------------
// Minimal incremental convex hull (O(n^2)) for generateObjPolyhedron
// Builds a list of triangular faces with outward-facing normals.
// ---------------------------------------------------------------------------

struct Face {
    int a{}, b{}, c{}; // indices into the global point array
    Vector3f normal{}; // outward normal
};

// Signed volume of tetrahedron formed by face (a,b,c) and point d
float signedVolume(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& d) {
    return dot(cross(b - a, c - a), d - a);
}

// Returns true if point p is strictly above face (i.e. on the outward side)
bool isVisible(const Face& f, const Vector3f& p, const std::vector<Vector3f>& pts) {
    return dot(f.normal, p - pts[f.a]) > 1e-7F;
}

std::vector<Face> buildConvexHull(const std::vector<Vector3f>& pts) {
    int n = static_cast<int>(pts.size());
    if (n < 4) {
        throw wrapRuntimeError("Need at least 4 non-coplanar points for a convex hull");
    }
    std::vector<Face> hull;

    // --- Build initial tetrahedron ---
    // Find 4 non-coplanar points
    int p0 = 0;
    int p1 = -1;
    int p2 = -1;
    int p3 = -1;

    for (int i = 1; i < n && p1 < 0; ++i) {
        if (norm(pts[i] - pts[p0]) > 1e-7F) {
            p1 = i;
        }
    }

    for (int i = 1; i < n && p2 < 0; ++i) {
        if (i != p1 && norm(cross(pts[i] - pts[p0], pts[p1] - pts[p0])) > 1e-7F) {
            p2 = i;
        }
    }

    for (int i = 1; i < n && p3 < 0; ++i) {
        if (i != p1 && i != p2 &&
            std::abs(dot(pts[i] - pts[p0], cross(pts[p1] - pts[p0], pts[p2] - pts[p0]))) > 1e-7F) {
            p3 = i;
        }
    }

    if (p1 < 0 || p2 < 0 || p3 < 0) {
        throw wrapRuntimeError("Points appear to be coplanar or collinear");
    }

    // Orient initial tetrahedron so all normals point outward
    // Use the centroid of the tetrahedron as the interior reference point
    Vector3f innerPoint = (pts[p0] + pts[p1] + pts[p2] + pts[p3]) * 0.25f;

    auto makeFace = [&](int a, int b, int c) -> Face {
        Vector3f n = normalized(cross(pts[b] - pts[a], pts[c] - pts[a]));
        // Flip if normal points inward
        if (dot(n, pts[a] - innerPoint) < 0) {
            n = n * -1.0F, std::swap(b, c);
        }
        return {a, b, c, n};
    };

    hull.push_back(makeFace(p0, p1, p2));
    hull.push_back(makeFace(p0, p1, p3));
    hull.push_back(makeFace(p0, p2, p3));
    hull.push_back(makeFace(p1, p2, p3));

    // --- Incrementally add remaining points ---
    for (int i = 0; i < n; ++i) {
        if (i == p0 || i == p1 || i == p2 || i == p3) {
            continue;
        }

        const Vector3f& p = pts[i];

        // Collect visible faces and horizon edges
        std::vector<bool> visible(hull.size(), false);
        bool anyVisible = false;
        for (int f = 0; f < (int)hull.size(); ++f) {
            if (isVisible(hull[f], p, pts)) {
                visible[f] = true;
                anyVisible = true;
            }
        }
        if (!anyVisible) {
            continue; // point already inside hull
        }

        // Find horizon edges: edges shared by exactly one visible face
        // Represent edge as ordered pair (min,max) + winding
        struct Edge { int a, b; };
        std::vector<Edge> horizon;
        for (int f = 0; f < (int)hull.size(); ++f) {
            if (!visible[f]) {
                continue;
            }
            int tri[3] = {hull[f].a, hull[f].b, hull[f].c};
            for (int e = 0; e < 3; ++e) {
                int ea = tri[e];
                int eb = tri[(e + 1) % 3];
                // Check if the adjacent face (sharing ea-eb reversed) is not visible
                bool adjVisible = false;
                for (int g = 0; g < (int)hull.size(); ++g) {
                    if (!visible[g]) {
                        continue;
                    }
                    if (g == f) {
                        continue;
                    }
                    int tri2[3] = {hull[g].a, hull[g].b, hull[g].c};
                    for (int e2 = 0; e2 < 3; ++e2) {
                        if (tri2[e2] == eb && tri2[(e2 + 1) % 3] == ea) {
                            adjVisible = true;
                            break;
                        }
                    }
                    if (adjVisible) {
                        break;
                    }
                }
                if (!adjVisible) {
                    horizon.push_back({ea, eb});
                }
            }
        }

        // Remove visible faces
        std::vector<Face> newHull;
        for (int f = 0; f < (int)hull.size(); ++f) {
            if (!visible[f]) {
                newHull.push_back(hull[f]);
            }
        }

        // Add new cone faces from horizon edges to the new point
        Vector3f innerPt = centroid([&]() {
            std::vector<Vector3f> tmp;
            for (const auto& ff : newHull) {
                tmp.push_back(pts[ff.a]);
                tmp.push_back(pts[ff.b]);
                tmp.push_back(pts[ff.c]);
            }
            return tmp;
        }());

        for (const auto& e : horizon) {
            Vector3f n = normalized(cross(pts[e.b] - pts[e.a], p - pts[e.a]));
            // Ensure outward orientation
            if (dot(n, pts[e.a] - innerPt) < 0) {
                n = n * -1.0f;
                newHull.push_back({e.b, e.a, i, n});
            } else {
                newHull.push_back({e.a, e.b, i, n});
            }
        }

        hull = std::move(newHull);
    }

    return hull;
}

std::string generateObjPolyhedron(const std::vector<float>& vertices) {
    if (vertices.size() % 3 != 0) {
        throw wrapRuntimeError("vertices must contain x,y,z triples");
    }
    if (vertices.size() < 12) {
        throw wrapRuntimeError("Need at least 4 points for a polyhedron");
    }

    const int n = static_cast<int>(vertices.size()) / 3;

    // Build Vector3f point list
    std::vector<Vector3f> pts;
    pts.reserve(n);
    for (int i = 0; i < n; ++i) {
        pts.push_back({vertices[3 * i], vertices[3 * i + 1], vertices[3 * i + 2]});
    }

    // Compute convex hull
    std::vector<Face> hull = buildConvexHull(pts);

    // -----------------------------------------------------------------------
    // Build deduplicated vertex list for the OBJ output.
    // We collect only the hull vertices (some input points may be interior).
    // -----------------------------------------------------------------------
    std::vector<Vector3f> outVerts;
    // Map from original index -> output index (1-based)
    std::unordered_map<int, int> indexMap;

    auto getOutIndex = [&](int origIdx) -> int {
        auto it = indexMap.find(origIdx);
        if (it != indexMap.end()) {
            return it->second;
        }
        outVerts.push_back(pts[origIdx]);
        int oi = 0;
        oi = static_cast<int>(outVerts.size()); // 1-based
        indexMap[origIdx] = oi;
        return oi;
    };

    // Pre-pass: register vertices in hull order for deterministic output
    for (const auto& f : hull) {
        getOutIndex(f.a);
        getOutIndex(f.b);
        getOutIndex(f.c);
    }

    std::ostringstream obj;
    obj << "o Polyhedron\n\n";

    // Vertices
    for (const auto& v : outVerts) {
        obj << "v " << v.x() << " " << v.y() << " " << v.z() << "\n";
    }

    obj << "\n# Faces\n";
    for (const auto& f : hull) {
        obj << "f " << indexMap[f.a] << " " << indexMap[f.b] << " " << indexMap[f.c] << "\n";
    }

    return obj.str();
}

}