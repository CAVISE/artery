#include <cavise/sionna/bridge/EnvironmentMeshGenerator.h>

namespace artery::sionna::meshes {

std::string generateObjSphere(float radius) {
    std::ostringstream oss;
    oss << "o Sphere\n";
    
    const int rings = 16, sectors = 32;
    
    for (int r = 0; r <= rings; ++r) {
        float phi = M_PI * r / rings;
        for (int s = 0; s <= sectors; ++s) {
            float theta = 2.0f * M_PI * s / sectors;
            float x = radius * sinf(phi) * cosf(theta);
            float y = radius * sinf(phi) * sinf(theta);
            float z = radius * cosf(phi);
            oss << "v " << x << " " << y << " " << z << "\n";
        }
    }
    
    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < sectors; ++s) {
            int i0 = r * (sectors + 1) + s;
            int i1 = i0 + sectors + 1;
            oss << "f " << i0+1 << " " << i1+1 << " " << (i1+1)+1 << "\n";
            oss << "f " << i0+1 << " " << (i1+1)+1 << " " << (i0+1)+1 << "\n";
        }
    }
    
    return oss.str();
}

}