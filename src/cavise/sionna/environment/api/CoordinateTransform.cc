#include "CoordinateTransform.h"

#include <omnetpp/cexception.h>

#include <artery/traci/Cast.h>
#include <libsumo/TraCIDefs.h>

#include <cavise/sionna/bridge/Compat.h>
#include <cavise/sionna/environment/Compat.h>
#include <cavise/sionna/environment/api/SionnaAPI.h>

#include <drjit/array.h>
#include <drjit/array_router.h>
#include <drjit/array_static.h>
#include <drjit/dynamic.h>
#include <drjit/matrix.h>
#include <drjit/tensor.h>
#include <drjit/util.h>

#include <mitsuba/core/spectrum.h>
#include <mitsuba/render/mesh.h>
#include <mitsuba/render/scene.h>

#include <algorithm>
#include <array>
#include <cmath>

using namespace artery::sionna;

namespace {

    [[noreturn]] void failRouting(CoordinateSystem to, CoordinateSystem from) {
        throw omnetpp::cRuntimeError("unsupported coordinate transform from %d to %d", static_cast<int>(from), static_cast<int>(to));
    }

} // namespace

AffineCoordinateTransform::AffineCoordinateTransform(
    const mi::Matrix3f& remap,
    const mi::Matrix2f& rotation,
    const mi::Vector3f& translation,
    const traci::Boundary& boundary,
    ISionnaAPI* api
)
    : remap_(remap)
    , rotation_(rotation)
    , translation_(translation)
    , api_(api)
    , boundary_(boundary)
{
    auto check = [](const auto& m) {
        auto det = toScalar<double>(drjit::det(m));
        if (std::fabs(det) < 1E-12) {
            throw omnetpp::cRuntimeError("matrix is singular");
        }
    };

    check(remap);
    check(rotation);

    inverseRemap_ = drjit::inverse(remap_);
    inverseRotation_ = drjit::inverse(rotation_);
}

mi::Vector3f AffineCoordinateTransform::convertCoordinates(CoordinateSystem from, CoordinateSystem to, const mi::Vector3f& v) {
    using c = CoordinateSystem;

    if (from == to) {
        return v;
    }

    switch (from) {
        case c::INET:
            return dispatchInet(to, v);
        case c::SIONNA_LOCAL:
            return dispatchLocalScene(to, v);
        case c::SIONNA_SCENE:
            return dispatchSionnaScene(to, v);
        case c::SUMO:
            return dispatchSumo(to, v);
        default:
            failRouting(to, from);
    }
}

mi::Vector3f AffineCoordinateTransform::dispatchInet(CoordinateSystem to, const mi::Vector3f& v) {
    using c = CoordinateSystem;

    auto sumo = [this](const mi::Vector3f& v) {
        auto pos = convert<Position, mi::Vector3f>(v);
        auto casted = position_cast(boundary_, pos);
        return convert<mi::Vector3f, libsumo::TraCIPosition>(casted);
    };

    switch (to) {
        case c::SUMO:
            return sumo(v);
        case c::SIONNA_SCENE:
            return convertCoordinates(c::SUMO, c::SIONNA_SCENE, sumo(v));
        case c::SIONNA_LOCAL:
            return convertCoordinates(c::SUMO, c::SIONNA_LOCAL, sumo(v));
        default:
            failRouting(to, c::INET);
    }
}

mi::Vector3f AffineCoordinateTransform::dispatchLocalScene(CoordinateSystem to, const mi::Vector3f& v) {
    using c = CoordinateSystem;

    auto sionna = [this](const mi::Vector3f& v) {
        return inverseRemap_ * v;
    };

    switch (to) {
        case c::SIONNA_SCENE:
            return sionna(v);
        case c::SUMO:
            return convertCoordinates(c::SIONNA_SCENE, c::SUMO, sionna(v));
        case c::INET:
            return convertCoordinates(c::SUMO, c::INET, convertCoordinates(c::SIONNA_SCENE, c::SUMO, sionna(v)));
        default:
            failRouting(to, c::SIONNA_LOCAL);
    }
}

mi::Vector3f AffineCoordinateTransform::dispatchSionnaScene(CoordinateSystem to, const mi::Vector3f& v) {
    using c = CoordinateSystem;

    auto toSumo = [this](const mi::Vector3f& v) {
        const auto shifted = v - translation_;
        const auto restored = inverseRotation_ * mi::Vector2f(shifted.x(), shifted.y());
        return mi::Vector3f(restored.x(), restored.y(), shifted.z());
    };

    auto toLocal = [this](const mi::Vector3f& v) {
        return remap_ * v;
    };

    switch (to) {
        case c::SUMO:
            return toSumo(v);
        case c::SIONNA_LOCAL:
            return toLocal(v);
        case c::INET:
            return convertCoordinates(c::SUMO, c::INET, toSumo(v));
        default:
            failRouting(to, c::SIONNA_SCENE);
    }
}

mi::Vector3f AffineCoordinateTransform::dispatchSumo(CoordinateSystem to, const mi::Vector3f& v) {
    using c = CoordinateSystem;

    auto fromSumo = [this](const mi::Vector3f& v) {
        const auto rotated = rotation_ * mi::Vector2f(v.x(), v.y());
        return mi::Vector3f(rotated.x(), rotated.y(), v.z()) + translation_;
    };

    auto toInet = [this](const mi::Vector3f& v) {
        auto pos = convert<libsumo::TraCIPosition, mi::Vector3f>(v);
        auto casted = position_cast(boundary_, pos);
        return convert<mi::Vector3f, Position>(casted);
    };

    switch (to) {
        case c::SIONNA_SCENE:
            return fromSumo(v);
        case c::SIONNA_LOCAL:
            return convertCoordinates(c::SIONNA_SCENE, c::SIONNA_LOCAL, fromSumo(v));
        case c::INET:
            return toInet(v);
        default:
            failRouting(to, c::SIONNA_SCENE);
    }
}

std::optional<mi::Point3f> AffineCoordinateTransform::orientationForPlane(const mi::TensorXf& plane) const {
    auto p0 = mi::Point3f(drjit::take(plane, mi::UInt32(0), 0).array());
    auto p1 = mi::Point3f(drjit::take(plane, mi::UInt32(1), 0).array());
    auto p2 = mi::Point3f(drjit::take(plane, mi::UInt32(2), 0).array());

    auto normal = drjit::cross(p1 - p0, p2 - p0);
    if (toScalar<double>(drjit::norm(normal)) < 1e-9) {
        return std::nullopt;
    }

    normal = drjit::normalize(normal);

    const mi::Vector3f yAxis(0.0, 1.0, 0.0);
    const auto dot = drjit::clip(drjit::dot(yAxis, normal), mi::Float(-1.0), mi::Float(1.0));
    const auto theta = drjit::acos(dot);

    auto axis = drjit::cross(yAxis, normal);
    if (toScalar<double>(drjit::norm(axis)) < 1e-9) {
        if (toScalar<double>(dot) > 0.0) {
            return mi::Point3f(0.0, 0.0, 0.0);
        }
        axis = mi::Vector3f(1.0, 0.0, 0.0);
    } else {
        axis = drjit::normalize(axis);
    }

    const auto q = mi::Quaternion4f(axis * drjit::sin(theta * 0.5), drjit::cos(theta * 0.5));
    return drjit::quat_to_euler(q);

}

void AffineCoordinateTransform::adjustVerticalComponent(py::SceneObject& object) {
    mi::Vector3f base = convertCoordinates(CoordinateSystem::SIONNA_LOCAL, CoordinateSystem::SIONNA_SCENE, object.position());

    const auto mesh = object.mesh();
    const auto& min = mesh->bbox().min;
    const auto& max = mesh->bbox().max;

    std::array<mi::Point3f, 8> bboxCorners = {
        mi::Point3f(min.x(), min.y(), min.z()),
        mi::Point3f(min.x(), min.y(), max.z()),
        mi::Point3f(min.x(), max.y(), min.z()),
        mi::Point3f(min.x(), max.y(), max.z()),
        mi::Point3f(max.x(), min.y(), min.z()),
        mi::Point3f(max.x(), min.y(), max.z()),
        mi::Point3f(max.x(), max.y(), min.z()),
        mi::Point3f(max.x(), max.y(), max.z()),
    };

    for (auto& point : bboxCorners) {
        point = convertCoordinates(CoordinateSystem::SIONNA_LOCAL, CoordinateSystem::SIONNA_SCENE, point);
    }

    std::sort(bboxCorners.begin(), bboxCorners.end(), [](const auto& a, const auto& b) {
        return toScalar<double>(a.z()) > toScalar<double>(b.z());
    });

    const std::array<mi::Point3f, 4> corners {
        bboxCorners[0],
        bboxCorners[1],
        bboxCorners[2],
        bboxCorners[3],
    };

    const std::array<std::size_t, 2> shape {4, 3};
    auto upperValues = drjit::concat(drjit::ravel(corners[0]), drjit::ravel(corners[1]));
    upperValues = drjit::concat(upperValues, drjit::ravel(corners[2]));
    upperValues = drjit::concat(upperValues, drjit::ravel(corners[3]));
    auto plane = mi::TensorXf(upperValues, shape.size(), shape.data());

    if (api_ == nullptr) {
        return;
    }

    auto scene = api_->miScene();
    const auto interactions = resolver_.resolve(plane, *scene);
    if (interactions.size() < 3) {
        return;
    }

    auto hitValues = drjit::ravel(interactions.front().p);
    for (std::size_t i = 1; i < interactions.size(); ++i) {
        hitValues = drjit::concat(hitValues, drjit::ravel(interactions[i].p));
    }

    const std::array<std::size_t, 2> hshape {interactions.size(), 3};
    auto hitPlane = mi::TensorXf(hitValues, hshape.size(), hshape.data());

    auto minZ = [&interactions]() {
        auto value = interactions.front().p.z();
        for (std::size_t i = 1; i < interactions.size(); ++i) {
            value = drjit::minimum(value, interactions[i].p.z());
        }
        return value;
    };

    auto pos = mi::Vector3f(base.x(), base.y(), minZ());
    object.setPosition(convertCoordinates(CoordinateSystem::SIONNA_SCENE, CoordinateSystem::SIONNA_LOCAL, pos));
    if (auto orientation = orientationForPlane(hitPlane)) {
        object.setOrientation(*orientation);
    }
}

std::vector<mi::SurfaceInteraction3f> MeshInteractionResolver::resolve(const mi::TensorXf& plane, mi::Scene& scene) const {
    using it = std::vector<std::string>::const_iterator;

    auto resolveOne = [&scene](mi::Ray3f ray, it itBegin, it itEnd) -> std::optional<mi::SurfaceInteraction3f> {
        std::vector<mi::SurfaceInteraction3f> interactions;

        while (true) {
            auto interaction = scene.ray_intersect(ray);
            if (!drjit::all_nested(interaction.is_valid())) {
                break;
            }

            interactions.push_back(interaction);
            ray.o = interaction.p + ray.d * mi::Float(1e-4f);
        }

        for (auto id = std::make_reverse_iterator(itEnd); id != std::make_reverse_iterator(itBegin); ++id) {
            const auto found = std::find_if(interactions.begin(), interactions.end(), [&id](const mi::SurfaceInteraction3f& interaction) {
                const auto* shape = drjit::slice(interaction.shape);
                return shape != nullptr && shape->id() == *id;
            });

            if (found != interactions.end()) {
                return *found;
            }
        }

        return std::nullopt;
    };

    std::vector<mi::SurfaceInteraction3f> interactions;
    auto down = mi::Vector3f(0.0, 0.0, -1.0);

    for (std::size_t row = 0; row < plane.shape(0); ++row) {
        auto o = mi::Point3f(drjit::take(plane, mi::UInt32(row), 0).array());
        auto ray = mi::Ray3f(o, down);

        if (auto interaction = resolveOne(ray, ids_.begin(), ids_.end())) {
            interactions.push_back(*interaction);
        }
    }

    return interactions;
}

MeshInteractionResolver& MeshInteractionResolver::addMesh(const std::string& id) {
    ids_.push_back(id);
    return *this;
}

MeshInteractionResolver& AffineCoordinateTransform::meshResolver() {
    return resolver_;
}
