#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

struct Vec3 {
    double x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(double X, double Y, double Z) : x(X), y(Y), z(Z) {}
};
static inline Vec3 operator+(const Vec3& a, const Vec3& b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}
static inline Vec3 operator-(const Vec3& a, const Vec3& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
static inline Vec3 operator*(const Vec3& a, double s) {
    return {a.x * s, a.y * s, a.z * s};
}
static inline double dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
static inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
static inline double norm(const Vec3& a) {
    return std::sqrt(dot(a, a));
}

struct Face {
    int a, b, c;
    Vec3 n;
};  // oriented triangle face with outward normal

// Signed distance from point p to plane of face (positive if in direction of normal)
static double signed_distance(const std::vector<Vec3>& P, const Face& f, const Vec3& p) {
    const Vec3& A = P[f.a];
    double d0 = -dot(f.n, A);
    return dot(f.n, p) + d0;
}

// Build initial non-degenerate tetrahedron indices (i0,i1,i2,i3)
static std::tuple<int, int, int, int> initial_tetra(const std::vector<Vec3>& P) {
    const int n = (int)P.size();
    if (n < 4)
        throw std::invalid_argument("Polyhedron needs at least 4 non-coplanar points");

    // i0 = any
    int i0 = 0;

    // i1 = farthest from i0
    int i1 = 1;
    double best = 0;
    for (int i = 1; i < n; ++i) {
        double d = norm(P[i] - P[i0]);
        if (d > best) {
            best = d;
            i1 = i;
        }
    }
    if (best == 0)
        throw std::invalid_argument("All points are identical");

    // i2 = farthest from line i0-i1
    Vec3 u = P[i1] - P[i0];
    int i2 = -1;
    best = 0;
    for (int i = 0; i < n; ++i)
        if (i != i0 && i != i1) {
            Vec3 w = P[i] - P[i0];
            double area2 = norm(cross(u, w));  // ~ distance to line * |u|
            if (area2 > best) {
                best = area2;
                i2 = i;
            }
        }
    if (i2 < 0 || best == 0)
        throw std::invalid_argument("All points are collinear");

    // i3 = farthest from plane (i0,i1,i2)
    Vec3 nrm = cross(P[i1] - P[i0], P[i2] - P[i0]);
    int i3 = -1;
    best = 0;
    for (int i = 0; i < n; ++i)
        if (i != i0 && i != i1 && i != i2) {
            double vol6 = std::abs(dot(nrm, P[i] - P[i0]));
            if (vol6 > best) {
                best = vol6;
                i3 = i;
            }
        }
    if (i3 < 0 || best == 0)
        throw std::invalid_argument("All points are coplanar");

    return {i0, i1, i2, i3};
}

// Ensure face normal points "outward" w.r.t. a reference point pref (not on face)
static Face make_face_oriented(const std::vector<Vec3>& P, int a, int b, int c, const Vec3& pref) {
    Vec3 n = cross(P[b] - P[a], P[c] - P[a]);
    // if normal points towards pref, flip
    if (dot(n, pref - P[a]) > 0) {
        std::swap(b, c);
        n = n * -1.0;
    }
    // normalize normal (not strictly needed for signed distance consistency)
    double ln = norm(n);
    if (ln > 0)
        n = n * (1.0 / ln);
    return {a, b, c, n};
}

// Quickhull-like expansion:
// Maintain current hull faces; for each point outside, find visible faces, remove them,
// and stitch the "horizon" with new faces to the point.
static void add_point_to_hull(const std::vector<Vec3>& P, int ip, std::vector<Face>& faces) {
    const double EPS = 1e-9;
    // 1) mark faces visible from P[ip]
    std::vector<char> visible(faces.size(), 0);
    for (size_t i = 0; i < faces.size(); ++i) {
        if (signed_distance(P, faces[i], P[ip]) > EPS)
            visible[i] = 1;
    }

    // 2) collect horizon edges: edges of visible faces that are not shared by another visible face
    struct Edge {
        int u, v;
    };
    auto canon = [](int a, int b) { return std::pair<int, int>(std::min(a, b), std::max(a, b)); };
    std::vector<Edge> boundary;
    // map edge->count among visible faces
    std::vector<std::pair<std::pair<int, int>, int>> edgemap;
    auto add_edge = [&](int u, int v) {
        auto key = canon(u, v);
        auto it = std::find_if(edgemap.begin(), edgemap.end(), [&](auto& kv) { return kv.first == key; });
        if (it == edgemap.end())
            edgemap.push_back({key, 1});
        else
            it->second++;
    };
    for (size_t i = 0; i < faces.size(); ++i)
        if (visible[i]) {
            const Face& f = faces[i];
            add_edge(f.a, f.b);
            add_edge(f.b, f.c);
            add_edge(f.c, f.a);
        }
    for (auto& kv : edgemap) {
        if (kv.second == 1) {  // appears once among visible faces -> horizon edge
            boundary.push_back({kv.first.first, kv.first.second});
        }
    }

    // 3) remove visible faces
    std::vector<Face> kept;
    kept.reserve(faces.size());
    for (size_t i = 0; i < faces.size(); ++i)
        if (!visible[i])
            kept.push_back(faces[i]);

    // 4) create new faces from boundary to ip
    // Need a reference point outside hull to orient normals consistently; use P[ip] plus normal
    for (auto e : boundary) {
        // orientation must be so that new face normal points outward (away from hull interior).
        // We orient triangle (e.u, e.v, ip) so that its normal points *away* from current hull.
        // A heuristic: orient using centroid as "interior" reference.
        Vec3 centroid(0, 0, 0);
        for (auto& f : kept)
            centroid = centroid + (P[f.a] + P[f.b] + P[f.c]) * (1.0 / 3.0);
        centroid = centroid * (1.0 / std::max(1.0, (double)kept.size()));
        Face nf = make_face_oriented(P, e.u, e.v, ip, centroid);
        kept.push_back(nf);
    }

    faces.swap(kept);
}

static std::vector<Face> convex_hull_3d(const std::vector<Vec3>& P) {
    auto [i0, i1, i2, i3] = initial_tetra(P);

    // Build initial 4 faces of the tetrahedron, oriented outward
    // Use the tetrahedron centroid as interior reference
    Vec3 C = (P[i0] + P[i1] + P[i2] + P[i3]) * 0.25;
    std::vector<Face> faces;
    faces.reserve(64);
    faces.push_back(make_face_oriented(P, i0, i1, i2, C));
    faces.push_back(make_face_oriented(P, i0, i3, i1, C));
    faces.push_back(make_face_oriented(P, i1, i3, i2, C));
    faces.push_back(make_face_oriented(P, i2, i3, i0, C));

    const double EPS = 1e-9;

    // Add remaining points
    const int n = (int)P.size();
    for (int i = 0; i < n; ++i) {
        if (i == i0 || i == i1 || i == i2 || i == i3)
            continue;

        // quick test: see if outside any face by > EPS
        bool outside = false;
        for (auto& f : faces) {
            if (signed_distance(P, f, P[i]) > EPS) {
                outside = true;
                break;
            }
        }
        if (!outside)
            continue;

        add_point_to_hull(P, i, faces);
    }
    return faces;
}

// =========================== PUBLIC API ===============================

// 1) From point cloud only: build convex hull and write triangulated OBJ
std::string write_polyhedron_obj_convex_hull(const std::vector<std::array<double, 3>>& points, const std::string& path = "polyhedron_tmp.obj") {
    if (points.size() < 4)
        throw std::invalid_argument("Need at least 4 non-coplanar points for a polyhedron");

    std::vector<Vec3> P;
    P.reserve(points.size());
    for (auto& q : points)
        P.emplace_back(q[0], q[1], q[2]);

    // Build hull
    auto faces = convex_hull_3d(P);

    // Output OBJ
    std::ofstream out(path);
    if (!out)
        throw std::runtime_error("Cannot open for writing: " + path);

    out << "# auto-generated polyhedron OBJ (convex hull)\n";
    out << "o Polyhedron\n";
    for (auto& v : P)
        out << "v " << v.x << " " << v.y << " " << v.z << "\n";
    for (auto& f : faces) {
        // Triangles, 1-based indices
        out << "f " << (f.a + 1) << " " << (f.b + 1) << " " << (f.c + 1) << "\n";
    }
    return path;
}

// 2) If you already have triangle faces (indices), just write them
std::string write_polyhedron_obj_indexed(
    const std::vector<std::array<double, 3>>& points, const std::vector<std::array<int, 3>>& tris, const std::string& path = "polyhedron_idx.obj") {
    std::ofstream out(path);
    if (!out)
        throw std::runtime_error("Cannot open for writing: " + path);

    out << "# auto-generated polyhedron OBJ (indexed)\n";
    out << "o Polyhedron\n";
    for (auto& q : points)
        out << "v " << q[0] << " " << q[1] << " " << q[2] << "\n";
    for (auto& t : tris)
        out << "f " << (t[0] + 1) << " " << (t[1] + 1) << " " << (t[2] + 1) << "\n";
    return path;
}
