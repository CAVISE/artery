#include <cavise/sionna/bridge/EnvironmentMeshGenerator.h>

namespace artery::sionna::meshes {

std::string generateObjCube(float x, float y, float z) {
    std::ostringstream oss;
    oss << "o Cube\n";

    float sx = x * 0.5F;
    float sy = y * 0.5F;
    float sz = z * 0.5F;

    oss << "v " << -sx << " " << -sy << " " << -sz << "\n"; // 1
    oss << "v " <<  sx << " " << -sy << " " << -sz << "\n"; // 2
    oss << "v " <<  sx << " " <<  sy << " " << -sz << "\n"; // 3
    oss << "v " << -sx << " " <<  sy << " " << -sz << "\n"; // 4
    oss << "v " << -sx << " " << -sy << " " <<  sz << "\n"; // 5
    oss << "v " <<  sx << " " << -sy << " " <<  sz << "\n"; // 6
    oss << "v " <<  sx << " " <<  sy << " " <<  sz << "\n"; // 7
    oss << "v " << -sx << " " <<  sy << " " <<  sz << "\n"; // 8
    
    // (-Z)
    oss << "f 1 2 3\nf 1 3 4\n";
    // (+Z)
    oss << "f 5 6 7\nf 5 7 8\n";
    // (-X)
    oss << "f 1 4 8\nf 1 8 5\n";
    // (+X)
    oss << "f 2 6 7\nf 2 7 3\n";
    // (-Y)
    oss << "f 1 5 6\nf 1 6 2\n";
    // (+Y)
    oss << "f 4 3 7\nf 4 7 8\n";
    
    return oss.str();
}

}