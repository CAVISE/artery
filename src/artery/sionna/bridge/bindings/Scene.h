#pragma once

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Capabilities.h>
#include <artery/sionna/bridge/Helpers.h>

#include <artery/sionna/bridge/bindings/Material.h>
#include <mitsuba/render/scene.h>

#include <nanobind/nanobind.h>

#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)
NAMESPACE_BEGIN(py)

namespace nb = nanobind;
using literals::operator""_a;

MI_VARIANT
class SionnaScene
    : public SionnaRtModuleBase
    , public CachedFetchCapability
    , public InitPythonClassCapability {
public:
    SIONNA_IMPORT_RENDER_TYPES(Scene)

    const char* className() const override {
        return "Scene";
    }

    SionnaScene(mitsuba::ref<Scene> miScene = nb::none(), bool removeDuplicateVertices = false) {
        init(
            "mi_scene"_a = std::move(miScene),
            "remove_duplicate_vertices"_a = removeDuplicateVertices);
    }

    const std::unordered_map<std::string, RadioMaterialBase<Float, Spectrum>>&
    radioMaterials(bool invalidate = false) const {
        if (invalidate) {
            radioMaterialCache_.reset();
        }

        if (!radioMaterialCache_) {
            nb::gil_scoped_acquire gil;

            nb::dict mats = nb::cast<nb::dict>(bound_->attr("radio_materials"));

            std::unordered_map<std::string, RadioMaterialBase<Float, Spectrum>> out;
            out.reserve(mats.size());

            for (auto kv : mats) {
                std::string key = nb::cast<std::string>(kv.first);
                nb::object val = nb::borrow<nb::object>(kv.second);
                out.emplace(std::move(key), RadioMaterialBase<Float, Spectrum>(std::move(val)));
            }

            radioMaterialCache_.emplace(std::move(out));
        }

        return *radioMaterialCache_;
    }

private:
    mutable std::optional<std::unordered_map<std::string, RadioMaterialBase<Float, Spectrum>>> radioMaterialCache_;
};

NAMESPACE_END(py)
NAMESPACE_END(sionna)
NAMESPACE_END(artery)
