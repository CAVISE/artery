#include <cavise/sionna/bridge/EnvironmentMeshGenerator.h>
#include <cmath>
#include <tuple>

namespace artery::sionna::meshes {

struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 cross(const Vec3& o) const {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }
    float norm() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3 normalized() const {
        float n = norm();
        if (n < 1e-9f) return {0, 0, 0};
        return {x / n, y / n, z / n};
    }
    bool operator==(const Vec3& o) const {
        return std::abs(x - o.x) < 1e-6f && std::abs(y - o.y) < 1e-6f &&
               std::abs(z - o.z) < 1e-6f;
    }
};

// Centroid of a list of points
Vec3 centroid(const std::vector<Vec3>& pts) {
    Vec3 c;
    for (const auto& p : pts) {
        c.x += p.x;
        c.y += p.y;
        c.z += p.z;
    }
    float n = static_cast<float>(pts.size());
    return {c.x / n, c.y / n, c.z / n};
}

// ---------------------------------------------------------------------------
// Minimal incremental convex hull (O(n^2)) for generateObjPolyhedron
// Builds a list of triangular faces with outward-facing normals.
// ---------------------------------------------------------------------------

struct Face {
    int a, b, c;    // indices into the global point array
    Vec3 normal;    // outward normal
};

// Signed volume of tetrahedron formed by face (a,b,c) and point d
float signedVolume(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d) {
    return (b - a).cross(c - a).dot(d - a);
}

// Returns true if point p is strictly above face (i.e. on the outward side)
bool isVisible(const Face& f, const Vec3& p, const std::vector<Vec3>& pts) {
    return f.normal.dot(p - pts[f.a]) > 1e-7f;
}

std::vector<Face> buildConvexHull(const std::vector<Vec3>& pts) {
    int n = static_cast<int>(pts.size());
    assert(n >= 4 && "Need at least 4 non-coplanar points for a convex hull");

    std::vector<Face> hull;

    // --- Build initial tetrahedron ---
    // Find 4 non-coplanar points
    int p0 = 0, p1 = -1, p2 = -1, p3 = -1;

    for (int i = 1; i < n && p1 < 0; ++i)
        if ((pts[i] - pts[p0]).norm() > 1e-7f)
            p1 = i;

    for (int i = 1; i < n && p2 < 0; ++i)
        if (i != p1 && (pts[i] - pts[p0]).cross(pts[p1] - pts[p0]).norm() > 1e-7f)
            p2 = i;

    for (int i = 1; i < n && p3 < 0; ++i)
        if (i != p1 && i != p2 &&
            std::abs((pts[i] - pts[p0]).dot((pts[p1] - pts[p0]).cross(pts[p2] - pts[p0]))) > 1e-7f)
            p3 = i;

    assert(p1 >= 0 && p2 >= 0 && p3 >= 0 && "Points appear to be coplanar or collinear");

    // Orient initial tetrahedron so all normals point outward
    // Use the centroid of the tetrahedron as the interior reference point
    Vec3 innerPoint = (pts[p0] + pts[p1] + pts[p2] + pts[p3]) * 0.25f;

    auto makeFace = [&](int a, int b, int c) -> Face {
        Vec3 n = (pts[b] - pts[a]).cross(pts[c] - pts[a]).normalized();
        // Flip if normal points inward
        if (n.dot(pts[a] - innerPoint) < 0)
            n = n * -1.0f, std::swap(b, c);
        return {a, b, c, n};
    };

    hull.push_back(makeFace(p0, p1, p2));
    hull.push_back(makeFace(p0, p1, p3));
    hull.push_back(makeFace(p0, p2, p3));
    hull.push_back(makeFace(p1, p2, p3));

    // --- Incrementally add remaining points ---
    for (int i = 0; i < n; ++i) {
        if (i == p0 || i == p1 || i == p2 || i == p3)
            continue;

        const Vec3& p = pts[i];

        // Collect visible faces and horizon edges
        std::vector<bool> visible(hull.size(), false);
        bool anyVisible = false;
        for (int f = 0; f < (int)hull.size(); ++f) {
            if (isVisible(hull[f], p, pts)) {
                visible[f] = true;
                anyVisible = true;
            }
        }
        if (!anyVisible) continue; // point already inside hull

        // Find horizon edges: edges shared by exactly one visible face
        // Represent edge as ordered pair (min,max) + winding
        struct Edge { int a, b; };
        std::vector<Edge> horizon;
        for (int f = 0; f < (int)hull.size(); ++f) {
            if (!visible[f]) continue;
            int tri[3] = {hull[f].a, hull[f].b, hull[f].c};
            for (int e = 0; e < 3; ++e) {
                int ea = tri[e], eb = tri[(e + 1) % 3];
                // Check if the adjacent face (sharing ea-eb reversed) is not visible
                bool adjVisible = false;
                for (int g = 0; g < (int)hull.size(); ++g) {
                    if (!visible[g]) continue;
                    if (g == f) continue;
                    int tri2[3] = {hull[g].a, hull[g].b, hull[g].c};
                    for (int e2 = 0; e2 < 3; ++e2) {
                        if (tri2[e2] == eb && tri2[(e2 + 1) % 3] == ea) {
                            adjVisible = true;
                            break;
                        }
                    }
                    if (adjVisible) break;
                }
                if (!adjVisible)
                    horizon.push_back({ea, eb});
            }
        }

        // Remove visible faces
        std::vector<Face> newHull;
        for (int f = 0; f < (int)hull.size(); ++f)
            if (!visible[f])
                newHull.push_back(hull[f]);

        // Add new cone faces from horizon edges to the new point
        Vec3 innerPt = centroid([&]() {
            std::vector<Vec3> tmp;
            for (const auto& ff : newHull) {
                tmp.push_back(pts[ff.a]);
                tmp.push_back(pts[ff.b]);
                tmp.push_back(pts[ff.c]);
            }
            return tmp;
        }());

        for (const auto& e : horizon) {
            Vec3 n = (pts[e.b] - pts[e.a]).cross(p - pts[e.a]).normalized();
            // Ensure outward orientation
            if (n.dot(pts[e.a] - innerPt) < 0) {
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
    assert(vertices.size() % 3 == 0 && "vertices must contain x,y,z triples");
    assert(vertices.size() >= 12 && "Need at least 4 points for a polyhedron");

    const int n = static_cast<int>(vertices.size()) / 3;

    // Build Vec3 point list
    std::vector<Vec3> pts;
    pts.reserve(n);
    for (int i = 0; i < n; ++i)
        pts.push_back({vertices[3 * i], vertices[3 * i + 1], vertices[3 * i + 2]});

    // Compute convex hull
    std::vector<Face> hull = buildConvexHull(pts);

    // -----------------------------------------------------------------------
    // Build deduplicated vertex list for the OBJ output.
    // We collect only the hull vertices (some input points may be interior).
    // -----------------------------------------------------------------------
    std::vector<Vec3> outVerts;
    // Map from original index -> output index (1-based)
    std::unordered_map<int, int> indexMap;

    auto getOutIndex = [&](int origIdx) -> int {
        auto it = indexMap.find(origIdx);
        if (it != indexMap.end()) return it->second;
        outVerts.push_back(pts[origIdx]);
        int oi = static_cast<int>(outVerts.size()); // 1-based
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
    for (const auto& v : outVerts)
        obj << "v " << v.x << " " << v.y << " " << v.z << "\n";

    obj << "\n# Faces\n";
    for (const auto& f : hull) {
        obj << "f " << indexMap[f.a] << " " << indexMap[f.b] << " " << indexMap[f.c] << "\n";
    }

    return obj.str();
}

}