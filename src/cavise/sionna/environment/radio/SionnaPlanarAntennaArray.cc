#include "SionnaPlanarAntennaArray.h"

#include <inet/common/InitStages.h>

using namespace artery::sionna;

Define_Module(SionnaPlanarAntennaArray);

namespace {

    std::optional<std::string> optionalStringParameter(omnetpp::cModule* module, const char* name) {
        auto value = module->par(name).stdstringValue();
        if (value.empty()) {
            return std::nullopt;
        }

        return value;
    }

} // namespace

void SionnaPlanarAntennaArray::initialize(int stage) {
    SionnaAntennaArray::initialize(stage);
    if (stage == inet::INITSTAGE_PHYSICAL_ENVIRONMENT_2) {
        py::PlanarArray array(
            par("numRows").intValue(),
            par("numCols").intValue(),
            par("pattern").stdstringValue(),
            static_cast<float>(par("verticalSpacing").doubleValue()),
            static_cast<float>(par("horizontalSpacing").doubleValue()),
            optionalStringParameter(this, "polarization"),
            optionalStringParameter(this, "polarizationModel"));
        setArray(std::move(array));
    }
}

std::ostream& SionnaPlanarAntennaArray::printToStream(std::ostream& stream, int level) const {
    stream << "SionnaPlanarAntennaArray";
    if (level <= PRINT_LEVEL_DETAIL) {
        stream << ", rows = " << par("numRows").intValue()
               << ", cols = " << par("numCols").intValue()
               << ", pattern = " << par("pattern").stdstringValue();
    }
    return SionnaAntennaArray::printToStream(stream, level);
}
