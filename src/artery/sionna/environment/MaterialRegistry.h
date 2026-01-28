#pragma once

#include <artery/sionna/bridge/Fwd.h>
#include <artery/sionna/bridge/Capabilities.h>
#include <artery/sionna/bridge/Helpers.h>

#include <artery/sionna/bridge/bindings/Scene.h>
#include <artery/sionna/environment/Material.h>

#include <inet/common/INETDefs.h>
#include <inet/common/Units.h>
#include <inet/environment/contract/IMaterial.h>
#include <inet/environment/contract/IMaterialRegistry.h>
#include <nanobind/nanobind.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

MI_VARIANT
class SionnaSceneMaterialRegistry : public inet::physicalenvironment::IMaterialRegistry
{
public:
    using RadioMaterial = RadioMaterial<Float, Spectrum>;

    SionnaSceneMaterialRegistry() = default;

    const inet::physicalenvironment::Material *getMaterial(const char *name) const override
    {
        return materials_.at(name).get();
    }

    void setMaterial(const std::string& name, std::unique_ptr<RadioMaterial> material) {
        materials_[name] = std::move(material);
    }

    friend struct nanobind::detail::type_caster<artery::sionna::SionnaSceneMaterialRegistry<Float, Spectrum>>;

private:
    std::unordered_map<std::string, std::unique_ptr<RadioMaterial>> materials_;
};

NAMESPACE_END(sionna)
NAMESPACE_END(artery)

NAMESPACE_BEGIN(nanobind)
NAMESPACE_BEGIN(detail)

MI_VARIANT
struct type_caster<artery::sionna::SionnaSceneMaterialRegistry<Float, Spectrum>> {
    using Type = artery::sionna::SionnaSceneMaterialRegistry<Float, Spectrum>;
    using RadioMaterial = artery::sionna::RadioMaterial<Float, Spectrum>;

    NB_TYPE_CASTER(Type, const_name<Type>())

    bool from_python(handle src, uint8_t flags, cleanup_list *cleanup) noexcept {
        make_caster<dict> dict_caster;
        if (!dict_caster.from_python(src, flags_for_local_caster<dict>(flags), cleanup)) {
            return false;
        }

        dict d = cast_t<dict>(dict_caster);
        for (const auto& [key, val] : d) {
            make_caster<std::string> str_caster;
            if (!str_caster.from_python(key, flags_for_local_caster<std::string>(flags), cleanup)) {
                return false;
            }

            const std::string name = cast_t<std::string>(str_caster);
            value.setMaterial(name, std::make_unique<RadioMaterial>(borrow<object>(val)));
        }

        return true;
    }

    static handle from_cpp(const Value& src, rv_policy, cleanup_list *) noexcept {
        nanobind::dict d;
        for (const auto& [name, matPtr] : src.materials()) {
            if (!matPtr) {
                continue;
            }
            d[name.c_str()] = matPtr->object();
        }
        return d.release();
    }
};

NAMESPACE_END(detail)
NAMESPACE_END(nanobind)
