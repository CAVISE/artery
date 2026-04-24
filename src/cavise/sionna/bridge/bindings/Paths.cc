#include "Paths.h"

#include <cavise/sionna/bridge/Compat.h>
#include <cavise/sionna/bridge/Helpers.h>

using namespace artery::sionna;

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

nanobind::object py::Paths::valid() const {
    return sionna::access<nanobind::object>(bound_, "valid");
}

std::tuple<nanobind::object, nanobind::object> py::Paths::coefficients() const {
    auto value = sionna::access<nanobind::tuple>(bound_, "a");
    return {
        nanobind::borrow<nanobind::object>(value[0]),
        nanobind::borrow<nanobind::object>(value[1])};
}

nanobind::object py::Paths::delays() const {
    return sionna::access<nanobind::object>(bound_, "tau");
}

nanobind::object py::Paths::sources() const {
    return sionna::access<nanobind::object>(bound_, "sources");
}

nanobind::object py::Paths::targets() const {
    return sionna::access<nanobind::object>(bound_, "targets");
}

double py::Paths::pathGain() const {
    nanobind::gil_scoped_acquire gil;

    auto dr = nanobind::module_::import_("drjit");
    auto op = nanobind::module_::import_("operator");

    auto mask = valid();
    auto [real, imag] = coefficients();

    auto realSquared = op.attr("mul")(real, real);
    auto imagSquared = op.attr("mul")(imag, imag);
    auto magnitudeSquared = op.attr("add")(realSquared, imagSquared);
    auto masked = op.attr("mul")(mask, magnitudeSquared);
    auto summed = dr.attr("sum")(masked.attr("array"));

    return sionna::toScalar<double>(nanobind::cast<maybe_diff_t<mitsuba::Resolve::Float>>(summed));
}

double py::Paths::pathGain(std::size_t rxIndex, std::size_t txIndex) const {
    nanobind::gil_scoped_acquire gil;

    auto dr = nanobind::module_::import_("drjit");
    auto op = nanobind::module_::import_("operator");
    auto builtins = nanobind::module_::import_("builtins");

    auto all = builtins.attr("slice")(nanobind::none(), nanobind::none(), nanobind::none());
    auto [real, imag] = coefficients();
    nanobind::object mask;
    nanobind::object realSlice;
    nanobind::object imagSlice;

    if (syntheticArray()) {
        auto maskSelector = nanobind::make_tuple(rxIndex, txIndex, all);
        auto coeffSelector = nanobind::make_tuple(rxIndex, all, txIndex, all, all);
        mask = op.attr("getitem")(valid(), maskSelector);
        realSlice = op.attr("getitem")(real, coeffSelector);
        imagSlice = op.attr("getitem")(imag, coeffSelector);
    } else {
        auto selector = nanobind::make_tuple(rxIndex, all, txIndex, all, all);
        mask = op.attr("getitem")(valid(), selector);
        realSlice = op.attr("getitem")(real, selector);
        imagSlice = op.attr("getitem")(imag, selector);
    }

    auto realSquared = op.attr("mul")(realSlice, realSlice);
    auto imagSquared = op.attr("mul")(imagSlice, imagSlice);
    auto magnitudeSquared = op.attr("add")(realSquared, imagSquared);
    auto masked = op.attr("mul")(mask, magnitudeSquared);
    auto summed = dr.attr("sum")(masked.attr("array"));

    return sionna::toScalar<double>(nanobind::cast<maybe_diff_t<mitsuba::Resolve::Float>>(summed));
}
