#include "Paths.h"

#include <cavise/sionna/bridge/Compat.h>
#include <cavise/sionna/bridge/Helpers.h>

#include <drjit/tensor.h>

#include <limits>

using namespace artery::sionna;

namespace {
    using TensorXu = drjit::Tensor<mitsuba::DynamicBuffer<mi::UInt32>>;

    template <typename Tensor>
    Tensor takeAxis(const Tensor& tensor, std::size_t index, int axis) {
        using Index = typename Tensor::Index;
        return drjit::take(tensor, Index(static_cast<std::uint32_t>(index)), axis);
    }

    template <typename Tensor>
    Tensor reshape(const Tensor& tensor, typename Tensor::Shape shape) {
        return Tensor(typename Tensor::Array(tensor.array()), std::move(shape));
    }

    template <typename Tensor>
    int lastAxis(const Tensor& tensor) {
        return static_cast<int>(tensor.shape().size()) - 1;
    }

    template <typename... Dims>
    mi::TensorXf::MaskType::Shape maskShape(Dims... dims) {
        mi::TensorXf::MaskType::Shape shape;
        (shape.push_back(static_cast<std::size_t>(dims)), ...);
        return shape;
    }

    double pathGainFromTensors(const mi::TensorXf::MaskType& mask, const mi::TensorXf& r, const mi::TensorXf& i) {
        const auto magnitude = r * r + i * i;
        const auto masked = mi::TensorXf(mask) * magnitude;
        return toScalar<double>(drjit::sum(masked.array()));
    }

    int classifyPathInteraction(const TensorXu& pathInteractions) {
        const auto none = py::InteractionTypes::none.value();
        for (std::size_t depth = 0; depth < pathInteractions.shape(0); ++depth) {
            const int interaction = toScalar<int>(takeAxis(pathInteractions, depth, 0).array());
            if (interaction != none) {
                return interaction;
            }
        }
        return none;
    }

} // namespace

const char* py::Paths::className() const {
    return "Paths";
}

py::Paths::Paths(nanobind::object obj) {
    WrapPythonClassCapability::init(std::move(obj));
}

std::size_t py::Paths::numTx() const {
    return sionna::access<std::size_t>(bound_, "num_tx");
}

std::size_t py::Paths::numRx() const {
    return sionna::access<std::size_t>(bound_, "num_rx");
}

bool py::Paths::syntheticArray() const {
    return sionna::access<bool>(bound_, "synthetic_array");
}

py::AntennaArray py::Paths::txArray() const {
    return sionna::access<py::AntennaArray>(bound_, "tx_array");
}

py::AntennaArray py::Paths::rxArray() const {
    return sionna::access<py::AntennaArray>(bound_, "rx_array");
}

mi::TensorXf::MaskType py::Paths::valid() const {
    return sionna::access<mi::TensorXf::MaskType>(bound_, "valid");
}

std::tuple<mitsuba::Resolve::TensorXf, mitsuba::Resolve::TensorXf> py::Paths::coefficients() const {
    return artery::sionna::access<std::tuple<mi::TensorXf, mi::TensorXf>>(bound_, "a");
}

double py::Paths::pathGain() const {
    auto mask = valid();
    auto [r, i] = coefficients();

    if (syntheticArray()) {
        mask = reshape(mask, maskShape(mask.shape(0), 1, mask.shape(1), 1, mask.shape(2)));
    }

    return pathGainFromTensors(mask, r, i);
}

double py::Paths::pathGain(std::size_t rxIndex, std::size_t txIndex) const {
    auto mask = valid();
    auto [r, i] = coefficients();

    if (syntheticArray()) {
        mask = takeAxis(takeAxis(mask, rxIndex, 0), txIndex, 0);
        mask = reshape(mask, maskShape(1, 1, mask.shape(0)));
    } else {
        mask = takeAxis(takeAxis(mask, rxIndex, 0), txIndex, 1);
    }

    r = takeAxis(takeAxis(r, rxIndex, 0), txIndex, 1);
    i = takeAxis(takeAxis(i, rxIndex, 0), txIndex, 1);

    return pathGainFromTensors(mask, r, i);
}

std::optional<int> py::Paths::strongestPathInteraction(std::size_t rxIndex, std::size_t txIndex) const {
    auto mask = valid();
    auto interactions = sionna::access<TensorXu>(bound_, "interactions");
    auto [r, i] = coefficients();

    if (syntheticArray()) {
        mask = takeAxis(takeAxis(mask, rxIndex, 0), txIndex, 0);
        interactions = takeAxis(takeAxis(interactions, rxIndex, 1), txIndex, 1);
    } else {
        mask = takeAxis(takeAxis(mask, rxIndex, 0), txIndex, 2);
        interactions = takeAxis(takeAxis(interactions, rxIndex, 1), txIndex, 4);
    }

    r = takeAxis(takeAxis(r, rxIndex, 0), txIndex, 1);
    i = takeAxis(takeAxis(i, rxIndex, 0), txIndex, 1);

    const std::size_t numPaths = mask.shape(lastAxis(mask));
    double strongestGain = -std::numeric_limits<double>::infinity();
    std::optional<std::size_t> strongestPath;

    for (std::size_t path = 0; path < numPaths; ++path) {
        const auto pathMask = takeAxis(mask, path, lastAxis(mask));
        if (toScalar<int>(drjit::sum(pathMask.array())) == 0) {
            continue;
        }

        const auto pathR = takeAxis(r, path, lastAxis(r));
        const auto pathI = takeAxis(i, path, lastAxis(i));

        if (const double gain = pathGainFromTensors(pathMask, pathR, pathI); gain > strongestGain) {
            strongestGain = gain;
            strongestPath = path;
        }
    }

    if (!strongestPath.has_value()) {
        return std::nullopt;
    }

    return classifyPathInteraction(takeAxis(interactions, strongestPath.value(), lastAxis(interactions)));
}
