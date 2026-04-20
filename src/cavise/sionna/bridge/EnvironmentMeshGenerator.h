#pragma once

#include <cavise/sionna/bridge/Fwd.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace artery::sionna::meshes {

std::string generateObjCube(float x, float y, float z);
std::string generateObjSphere(float radius);
std::string generateObjPrism(float height, const std::vector<float>& vertices);
std::string generateObjPolyhedron(const std::vector<float>& vertices);

} // namespace artery::sionna::meshes
