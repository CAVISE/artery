#pragma once

#include <nanobind/nanobind.h>

#include <cavise/sionna/bridge/bindings/Modules.h>
#include <cavise/sionna/bridge/capabilities/Calling.h>

#include <optional>
#include <string>

namespace artery::sionna::py {

    class SIONNA_BRIDGE_API AntennaArray
        : public SionnaRtModule
        , public InitPythonClassCapability {
    public:
        const char* className() const override;

        AntennaArray() = default;
        explicit AntennaArray(nanobind::object obj);

        std::size_t numAntennas() const;
        std::size_t arraySize() const;
    };

    class SIONNA_BRIDGE_API PlanarArray
        : public AntennaArray {
    public:
        const char* className() const override;

        PlanarArray() = default;
        explicit PlanarArray(nanobind::object obj);
        PlanarArray(
            int numRows,
            int numCols,
            const std::string& pattern,
            float verticalSpacing = 0.5f,
            float horizontalSpacing = 0.5f,
            std::optional<std::string> polarization = std::nullopt,
            std::optional<std::string> polarizationModel = std::nullopt);
    };

} // namespace artery::sionna::py
