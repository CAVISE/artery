#pragma once

#include <nanobind/nanobind.h>

#include <cavise/sionna/bridge/bindings/AntennaArray.h>
#include <cavise/sionna/bridge/bindings/Modules.h>
#include <cavise/sionna/bridge/capabilities/Calling.h>

namespace artery::sionna::py {

    class SIONNA_BRIDGE_API Paths
        : public SionnaRtModule
        , public InitPythonClassCapability {
    public:
        const char* className() const override;

        Paths() = default;
        explicit Paths(nanobind::object obj);

        std::size_t numTx() const;
        std::size_t numRx() const;
        bool syntheticArray() const;

        AntennaArray txArray() const;
        AntennaArray rxArray() const;

        nanobind::object valid() const;
        std::tuple<nanobind::object, nanobind::object> coefficients() const;
        nanobind::object delays() const;
        nanobind::object sources() const;
        nanobind::object targets() const;

        double pathGain() const;
        double pathGain(std::size_t rxIndex, std::size_t txIndex) const;
    };

} // namespace artery::sionna::py
