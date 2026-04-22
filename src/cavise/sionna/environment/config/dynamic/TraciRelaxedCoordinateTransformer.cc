#include "TraciRelaxedCoordinateTransformer.h"

#include <cavise/sionna/bridge/Fwd.h>
#include <cavise/sionna/bridge/Compat.h>
#include <cavise/sionna/environment/config/scenes/IStaticSceneProvider.h>

#include <drjit/array.h>
#include <drjit/array_router.h>
#include <drjit/dynamic.h>
#include <drjit/matrix.h>
#include <drjit/tensor.h>
#include <omnetpp/cexception.h>
#include <inet/common/InitStages.h>
#include <mitsuba/render/mesh.h>
#include <mitsuba/render/scene.h>

#include <algorithm>
#include <cmath>
#include <optional>

using namespace artery::sionna;

Define_Module(TraciRelaxedCoordinateTransformer);

namespace {

    class PlaneTransformer {
    public:
        static mitsuba::Resolve::Matrix4f fromHits(const std::vector<mitsuba::Resolve::Point3f>& hits) {
            if (hits.size() >= 3) {
                return threeHits(hits);
            } else if (hits.size() == 2) {
                return twoHits(hits);
            } else if (hits.size() == 1) {
                return oneHit(hits);
            }

            throw omnetpp::cRuntimeError("expected at least 1 hit, got %zu", hits.size());
        }

        static mitsuba::Resolve::Matrix4f threeHits(const std::vector<mitsuba::Resolve::Point3f>& hits) {
            const mitsuba::Resolve::Point3f& p0 = hits[0];
            const mitsuba::Resolve::Point3f& p1 = hits[1];
            const mitsuba::Resolve::Point3f& p2 = hits[2];

            auto normal = drjit::normalize(drjit::cross(p1 - p0, p2 - p0));
            const mitsuba::Resolve::Vector3f up(0.0, 0.0, 1.0);

            if (toScalar<double>(drjit::dot(normal, up)) < 0.0) {
                normal = -normal;
            }

            if (std::abs(toScalar<double>(drjit::dot(normal, up))) < 1e-9) {
                std::vector<mitsuba::Resolve::Point3f> reduced{p0, p1};
                return twoHits(reduced);
            }

            auto support = drjit::dot(normal, p0);
            for (const auto& hit : hits) {
                support = drjit::maximum(support, drjit::dot(normal, hit));
            }

            auto plane = drjit::zeros<mitsuba::Resolve::Matrix4f>();
            for (std::size_t i = 0; i < 3; ++i) {
                auto hit = hits[i];
                hit.z() = (support - normal.x() * hit.x() - normal.y() * hit.y()) / normal.z();

                plane[i][0] = hit.x();
                plane[i][1] = hit.y();
                plane[i][2] = hit.z();
            }

            // Keep a valid 4th row so callers can safely read all rows.
            plane[3] = plane[2];

            return plane;
        }

        static mitsuba::Resolve::Matrix4f twoHits(const std::vector<mitsuba::Resolve::Point3f>& hits) {
            const mitsuba::Resolve::Point3f& p0 = hits[0];
            const mitsuba::Resolve::Point3f& p1 = hits[1];
            const mitsuba::Resolve::Vector3f up(0.0, 0.0, 1.0);

            auto tangent = p1 - p0;
            if (toScalar<double>(drjit::norm(tangent)) < 1e-9) {
                std::vector<mitsuba::Resolve::Point3f> reduced{p0};
                return oneHit(reduced);
            }

            tangent = drjit::normalize(tangent);
            auto side = drjit::cross(up, tangent);

            if (toScalar<double>(drjit::norm(side)) < 1e-9) {
                side = mitsuba::Resolve::Vector3f(1.0, 0.0, 0.0);
            } else {
                side = drjit::normalize(side);
            }

            auto normal = drjit::normalize(drjit::cross(tangent, side));
            if (toScalar<double>(drjit::dot(normal, up)) < 0.0) {
                normal = -normal;
            }

            if (std::abs(toScalar<double>(drjit::dot(normal, up))) < 1e-9) {
                std::vector<mitsuba::Resolve::Point3f> reduced{p0};
                return oneHit(reduced);
            }

            auto support = drjit::maximum(drjit::dot(normal, p0), drjit::dot(normal, p1));

            auto plane = drjit::zeros<mitsuba::Resolve::Matrix4f>();
            for (std::size_t i = 0; i < 2; ++i) {
                auto hit = hits[i];
                hit.z() = (support - normal.x() * hit.x() - normal.y() * hit.y()) / normal.z();

                plane[i][0] = hit.x();
                plane[i][1] = hit.y();
                plane[i][2] = hit.z();
            }

            // Keep valid padded rows for callers that iterate 4 rows.
            plane[2] = plane[1];
            plane[3] = plane[1];

            return plane;
        }

        static mitsuba::Resolve::Matrix4f oneHit(const std::vector<mitsuba::Resolve::Point3f>& hits) {
            auto plane = drjit::zeros<mitsuba::Resolve::Matrix4f>();
            const auto& hit = hits[0];

            plane[0][0] = hit.x();
            plane[0][1] = hit.y();
            plane[0][2] = hit.z();

            // Keep valid padded rows for callers that iterate 4 rows.
            plane[1] = plane[0];
            plane[2] = plane[0];
            plane[3] = plane[0];

            return plane;
        }
    };

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

std::vector<mitsuba::Resolve::Point3f> TraciRelaxedCoordinateTransformer::discoverRoadHits(const mitsuba::Resolve::Matrix4f& upperPoints) const {
    if (!scene_.has_value()) {
        throw omnetpp::cRuntimeError("cannot discover road hits before scene is bound");
    }

    // NOTE: The idea here is to discover actual physical anchor points under vehicle. SUMO cars
    // have no Z coordinate, and our sim may run with uneven terrain, making vehicles fly or submerge under
    // roads. Here, we take the most likely 4 valid points from upper bbox rect and casting rays down finding actual terrain.
    std::vector<mitsuba::Resolve::Point3f> hits;

    // NOTE: We expect at most 4 points.
    hits.reserve(upperPoints.size());

    mitsuba::Resolve::Ray3f down;
    mitsuba::Resolve::Ray3f up;
    down.d = toLocalScene(mitsuba::Resolve::Vector3f(0.0, 0.0, -1.0));
    up.d = toLocalScene(mitsuba::Resolve::Vector3f(0.0, 0.0, 1.0));

    auto miScene = scene_->miScene();
    for (std::size_t i = 0; i < upperPoints.size(); ++i) {
        // We have forth coordinate here, reduce it.
        const auto& upper = upperPoints[i];
        const auto origin = toLocalScene(mitsuba::Resolve::Point3f(upper.x(), upper.y(), upper.z()));

        down.o = origin;
        up.o = origin;

        // NOTE: We do not use direct intersections for road meshes, as they may be arbitrary.
        auto downHit = miScene->ray_intersect(down);
        auto upHit = miScene->ray_intersect(up);

        const bool downValid = drjit::all_nested(downHit.is_valid());
        const bool upValid = drjit::all_nested(upHit.is_valid());

        if (downValid && upValid) {
            const auto downDist = toScalar<double>(drjit::norm(downHit.p - origin));
            const auto upDist = toScalar<double>(drjit::norm(upHit.p - origin));
            hits.push_back(fromLocalScene(downDist <= upDist ? downHit.p : upHit.p));
        } else if (downValid) {
            hits.push_back(fromLocalScene(downHit.p));
        } else if (upValid) {
            hits.push_back(fromLocalScene(upHit.p));
        }
    }

    return hits;
}

std::optional<mitsuba::Resolve::Point3f> TraciRelaxedCoordinateTransformer::orientationForPlane(const mitsuba::Resolve::Matrix4f& plane, const mitsuba::Resolve::Vector3f& localVelocity) const {
    const mitsuba::Resolve::Vector3f p0(plane[0].x(), plane[0].y(), plane[0].z());
    const mitsuba::Resolve::Vector3f p1(plane[1].x(), plane[1].y(), plane[1].z());
    const mitsuba::Resolve::Vector3f p2(plane[2].x(), plane[2].y(), plane[2].z());
    const mitsuba::Resolve::Vector3f p3(plane[3].x(), plane[3].y(), plane[3].z());

    std::optional<mitsuba::Resolve::Vector3f> normal;
    const auto n012 = drjit::cross(p1 - p0, p2 - p0);
    if (toScalar<double>(drjit::norm(n012)) >= 1e-9) {
        normal = drjit::normalize(n012);
    } else {
        const auto n013 = drjit::cross(p1 - p0, p3 - p0);
        if (toScalar<double>(drjit::norm(n013)) >= 1e-9) {
            normal = drjit::normalize(n013);
        }
    }

    if (!normal.has_value()) {
        return std::nullopt;
    }

    const mitsuba::Resolve::Vector3f upCanonical (0.0, 0.0, 1.0);
    if (toScalar<double>(drjit::dot(normal.value(), upCanonical)) < 0.0) {
        normal = -normal.value();
    }

    auto forward = fromLocalScene(localVelocity);
    if (toScalar<double>(drjit::norm(forward)) < 1e-9) {
        return std::nullopt;
    }

    forward = forward - drjit::dot(forward, normal.value()) * normal.value();
    if (toScalar<double>(drjit::norm(forward)) < 1e-9) {
        return std::nullopt;
    }

    forward = drjit::normalize(forward);
    auto side = drjit::cross(normal.value(), forward);

    if (toScalar<double>(drjit::norm(side)) < 1e-9) {
        return std::nullopt;
    }

    side = drjit::normalize(side);

    const double r11 = toScalar<double>(forward.x());
    const double r21 = toScalar<double>(forward.y());
    const double r31 = toScalar<double>(forward.z());
    const double r32 = toScalar<double>(side.z());
    const double r33 = toScalar<double>(normal.value().z());

    const double beta = std::asin(std::clamp(-r31, -1.0, 1.0));
    const double alpha = std::atan2(r21, r11);
    const double gamma = std::atan2(r32, r33);

    return mitsuba::Resolve::Point3f(alpha, beta, gamma);
}

void TraciRelaxedCoordinateTransformer::adjust(py::SceneObject object) const {
    mitsuba::Resolve::Vector3f base = fromLocalScene(object.position());

    const auto mesh = object.mesh();
    const auto& min = mesh->bbox().min;
    const auto& max = mesh->bbox().max;

    mitsuba::Resolve::Matrix4f upper {};
    std::array<mitsuba::Resolve::Vector3f, 8> corners;
    std::size_t cornerIndex = 0;
    for (const auto lx : {min.x(), max.x()}) {
        for (const auto ly : {min.y(), max.y()}) {
            for (const auto lz : {min.z(), max.z()}) {
                corners[cornerIndex++] = fromLocalScene(mitsuba::Resolve::Vector3f(lx, ly, lz));
            }
        }
    }

    std::array<std::size_t, 8> order {};
    for (std::size_t i = 0; i < order.size(); ++i) {
        order[i] = i;
    }

    std::sort(order.begin(), order.end(), [&corners](std::size_t a, std::size_t b) {
        return toScalar<double>(corners[a].z()) < toScalar<double>(corners[b].z());
    });

    for (std::size_t i = 0; i < 4; ++i) {
        const auto& up = corners[order[order.size() - 1 - i]];
        upper[i][0] = up.x();
        upper[i][1] = up.y();
        upper[i][2] = up.z();
    }

    const auto hits = discoverRoadHits(upper);
    if (hits.empty()) {
        // NOTE: skip modifications if no valid support points were found.
        return;
    }

    const auto fitted = PlaneTransformer::fromHits(hits);

    // Position-only correction in canonical coordinates: preserve X/Y and set vertical Z to fitted support.
    auto minZ = [](const mitsuba::Resolve::Matrix4f& m) {
        auto value = m[0].z();
        for (std::size_t i = 1; i < 4; ++i) {
            value = drjit::minimum(value, m[i].z());
        }
        return value;
    };

    object.setPosition(toLocalScene(mitsuba::Resolve::Vector3f(base.x(), base.y(), minZ(fitted))));
    // NOTE: Keep vehicle heading orientation from TraCI update path for now.
    // Relaxed slope orientation can tilt meshes into the road; revisit when
    // orientation model is stabilized.
}
