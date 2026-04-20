#include "TraciRelaxedCoordinateTransformer.h"

#include <cavise/sionna/bridge/Fwd.h>
#include <cavise/sionna/bridge/Compat.h>
#include <cavise/sionna/environment/config/scenes/IStaticSceneProvider.h>

#include <drjit/matrix.h>
#include <inet/common/InitStages.h>
#include <mitsuba/render/mesh.h>
#include <mitsuba/render/scene.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <variant>

using namespace artery::sionna;

Define_Module(TraciRelaxedCoordinateTransformer);

namespace {

    struct RoadHit {
        mitsuba::Resolve::Point3f point;
        mitsuba::Resolve::Vector3f normal;
        double distance = std::numeric_limits<double>::infinity();
    };

    struct Vector3d {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
    };

    Vector3d vector(const mitsuba::Resolve::Vector3f& v) {
        return {
            toScalar<double>(v.x()),
            toScalar<double>(v.y()),
            toScalar<double>(v.z())};
    }

    Vector3d normal(const mitsuba::Resolve::Normal3f& n) {
        return {
            toScalar<double>(n.x()),
            toScalar<double>(n.y()),
            toScalar<double>(n.z())};
    }

    Vector3d vector(const mitsuba::Resolve::Normal3f& n) {
        return normal(n);
    }

    double dot(const Vector3d& a, const Vector3d& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    Vector3d cross(const Vector3d& a, const Vector3d& b) {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
    }

    Vector3d operator-(const Vector3d& a, const Vector3d& b) {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }

    Vector3d operator*(double s, const Vector3d& v) {
        return {s * v.x, s * v.y, s * v.z};
    }

    double norm(const Vector3d& v) {
        return std::sqrt(dot(v, v));
    }

    std::optional<Vector3d> normalized(const Vector3d& v) {
        const double n = norm(v);
        if (n < 1e-9) {
            return std::nullopt;
        }

        return Vector3d{v.x / n, v.y / n, v.z / n};
    }

    mitsuba::Resolve::Point3f orientationFromBasis(const Vector3d& forward, const Vector3d& side, const Vector3d& up) {
        const double r11 = forward.x;
        const double r21 = forward.y;
        const double r31 = forward.z;
        const double r32 = side.z;
        const double r33 = up.z;

        const double beta = std::asin(std::clamp(-r31, -1.0, 1.0));
        const double alpha = std::atan2(r21, r11);
        const double gamma = std::atan2(r32, r33);

        return {alpha, beta, gamma};
    }

    std::optional<RoadHit> selectClosest(std::optional<RoadHit> a, std::optional<RoadHit> b) {
        if (!a) {
            return b;
        }
        if (!b) {
            return a;
        }
        return a->distance <= b->distance ? a : b;
    }

    template <typename Intersect>
    std::optional<RoadHit> intersectVertical(
        const mitsuba::Resolve::Point3f& origin,
        const mitsuba::Resolve::Vector3f& direction,
        Intersect&& intersect) {
        mitsuba::Resolve::Ray3f ray;
        ray.o = origin;
        ray.d = direction;

        const auto hit = intersect(ray);
        if (!drjit::all_nested(hit.is_valid())) {
            return std::nullopt;
        }

        auto n = hit.n;
        const Vector3d up{0.0, 1.0, 0.0};
        auto nd = normal(n);
        if (dot(nd, up) < 0.0) {
            n = -n;
            nd = {-nd.x, -nd.y, -nd.z};
        }

        return RoadHit{
            hit.p,
            mitsuba::Resolve::Vector3f(n.x(), n.y(), n.z()),
            std::abs(toScalar<double>(hit.p.y()) - toScalar<double>(origin.y()))};
    }

} // namespace

int TraciRelaxedCoordinateTransformer::numInitStages() const {
    return TraciCoordinateTransformer::numInitStages();
}

void TraciRelaxedCoordinateTransformer::initialize(int stage) {
    TraciCoordinateTransformer::initialize(stage);
    if (stage != inet::INITSTAGE_PHYSICAL_ENVIRONMENT) {
        return;
    }

    if (roadMeshName_ = par("roadMeshName").stdstringValue(); roadMeshName_.empty()) {
        EV_WARN << "Empty road meshes make finding Z coordinate highly inefficient: please specify road mesh or replace this module";
    }
}

void TraciRelaxedCoordinateTransformer::bindScene(py::SionnaScene scene) {
    scene_.emplace(std::move(scene));
}

void TraciRelaxedCoordinateTransformer::adjust(py::SceneObject object) const {
    if (!scene_) {
        throw omnetpp::cRuntimeError("TraciRelaxedCoordinateTransformer cannot adjust object before scene is bound");
    }

    const auto basePoint = object.position();
    mitsuba::Resolve::Vector3f base{basePoint.x(), basePoint.y(), basePoint.z()};

    // The imported scene is Y-up in Sionna-local coordinates.
    const mitsuba::Resolve::Vector3f up{0.0, 1.0, 0.0};
    const mitsuba::Resolve::Vector3f down{0.0, -1.0, 0.0};

    // Compute bounding box and take lower global Y coordinate.
    const auto mesh = object.mesh();
    const auto lower = toScalar<double>(mesh->bbox().min.y());

    // Preserve the current object anchor point above its lowest mesh point.
    const auto offset = toScalar<double>(base.y()) - lower;
    ASSERT(offset >= 0);

    const mitsuba::Resolve::Point3f rayOrigin{base.x(), lower, base.z()};
    std::optional<RoadHit> hit;
    if (!roadMeshName_.empty()) {
        const auto roadObject = scene_->get(roadMeshName_);
        if (const auto* road = std::get_if<py::SceneObject>(&roadObject); road) {
            auto roadMesh = road->mesh();
            auto intersectRoad = [roadMesh](const mitsuba::Resolve::Ray3f& ray) {
                auto pi = roadMesh->ray_intersect_preliminary(ray);
                return pi.compute_surface_interaction(ray);
            };
            hit = selectClosest(
                intersectVertical(rayOrigin, down, intersectRoad),
                intersectVertical(rayOrigin, up, intersectRoad));
        }
    }

    if (!hit) {
        auto miScene = scene_->miScene();
        auto intersectScene = [miScene](const mitsuba::Resolve::Ray3f& ray) {
            return miScene->ray_intersect(ray);
        };
        hit = selectClosest(
            intersectVertical(rayOrigin, down, intersectScene),
            intersectVertical(rayOrigin, up, intersectScene));
    }

    // In case there was no mesh - just return current position. It's probably quite bad
    // that cars are flying, but it's not for this module to handle that.
    if (!hit) {
        return;
    }

    object.setPosition({
        base.x(),
        hit->point.y() + offset,
        base.z()});

    const auto velocity = vector(object.velocity());
    const auto roadNormal = normalized(vector(hit->normal));
    if (!roadNormal) {
        return;
    }

    const auto tangent = normalized(velocity - dot(velocity, *roadNormal) * *roadNormal);
    if (!tangent) {
        return;
    }

    const auto side = normalized(cross(*roadNormal, *tangent));
    if (!side) {
        return;
    }

    object.setOrientation(orientationFromBasis(*tangent, *side, *roadNormal));
}
