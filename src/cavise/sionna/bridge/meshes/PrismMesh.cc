#include <cavise/sionna/bridge/EnvironmentMeshGenerator.h>

namespace artery::sionna::meshes {

std::string generateObjPrism(float height, const std::vector<float>& vertices) {
    const int n = static_cast<int>(vertices.size()) / 2; // number of base polygon points

    std::ostringstream obj;
    obj << "o Prism\n";

    // -----------------------------------------------------------------------
    // Vertices
    // Bottom ring (z = 0): indices 1..n
    // Top ring    (z = h): indices n+1..2n
    // -----------------------------------------------------------------------
    obj << "# Bottom ring (z = 0)\n"; // Debug
    for (int i = 0; i < n; ++i)
        obj << "v " << vertices[2 * i] << " " << vertices[2 * i + 1] << " 0\n";

    obj << "\n# Top ring (z = " << height << ")\n"; // Debug
    for (int i = 0; i < n; ++i)
        obj << "v " << vertices[2 * i] << " " << vertices[2 * i + 1] << " " << height << "\n";

    obj << "\n";

    // OBJ indices are 1-based
    // Bottom ring: vertex i   -> OBJ index (i+1)
    // Top ring:    vertex i   -> OBJ index (n + i + 1)

    // -----------------------------------------------------------------------
    // Bottom face (fan triangulation, reversed winding for downward normal)
    // -----------------------------------------------------------------------
    obj << "# Bottom face\n";
    for (int i = 1; i < n - 1; ++i)
        obj << "f " << 1 << " " << (i + 2) << " " << (i + 1) << "\n";

    // -----------------------------------------------------------------------
    // Top face (fan triangulation, normal upward)
    // -----------------------------------------------------------------------
    obj << "\n# Top face\n";
    for (int i = 1; i < n - 1; ++i)
        obj << "f " << (n + 1) << " " << (n + i + 1) << " " << (n + i + 2) << "\n";

    // -----------------------------------------------------------------------
    // Side faces (quads split into two triangles per edge of the base polygon)
    // -----------------------------------------------------------------------
    obj << "\n# Side faces\n";
    for (int i = 0; i < n; ++i) {
        int bot0 = i + 1;          // OBJ index, bottom vertex i
        int bot1 = (i + 1) % n + 1; // OBJ index, bottom vertex i+1
        int top0 = n + i + 1;      // OBJ index, top vertex i
        int top1 = n + (i + 1) % n + 1; // OBJ index, top vertex i+1

        // Two triangles per quad, outward normals
        obj << "f " << bot0 << " " << bot1 << " " << top1 << "\n";
        obj << "f " << bot0 << " " << top1 << " " << top0 << "\n";
    }

    return obj.str();
}

}