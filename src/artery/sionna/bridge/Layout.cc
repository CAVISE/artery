#include "Layout.h"

#include <artery/sionna/bridge/Fwd.h>

using namespace artery::sionna;

namespace
{

std::string operator-(const std::string& parent, const std::string& component)
{
    return format("%s.%s", parent.c_str(), component.c_str());
}

}  // namespace

const std::string ModuleLayout::sionna = "sionna";
const std::string ModuleLayout::sionnaRt = sionna - "rt";
const std::string ModuleLayout::sionnaConstants = sionnaRt - "constants";
