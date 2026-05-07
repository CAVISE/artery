#include "PhysicalEnvironment.h"

#include <inet/environment/common/MaterialRegistry.h>

#include <cavise/sionna/environment/api/DynamicSceneConfig.h>
#include <cavise/sionna/environment/api/CoordinateTransform.h>
#include <cavise/sionna/environment/api/IDConverter.h>

#include <drjit-core/jit.h>
#include <nanobind/nanobind.h>

#include <omnetpp/cmodule.h>
#include <omnetpp/cexception.h>
#include <omnetpp/cxmlelement.h>
#include <traci/API.h>
#include <traci/BasicNodeManager.h>

using namespace artery::sionna;

Define_Module(PhysicalEnvironment);

namespace {

    template <typename Matrix>
    Matrix parseMatrix(omnetpp::cXMLElement* data) {
        Matrix matrix;
        for (const auto& item : data->getElementsByTagName("element")) {
            std::size_t row = std::stoull(item->getAttribute("row"));
            std::size_t column = std::stoull(item->getAttribute("column"));
            matrix[row][column] = std::stod(item->getNodeValue());
        }

        return matrix;
    }

    mi::Vector3f parseTranslationVector(omnetpp::cXMLElement* data) {
        return {
            std::stod(data->getFirstChildWithTag("x")->getNodeValue()),
            std::stod(data->getFirstChildWithTag("y")->getNodeValue()),
            std::stod(data->getFirstChildWithTag("z")->getNodeValue())};
    }

} // namespace

int PhysicalEnvironment::numInitStages() const {
    return inet::NUM_INIT_STAGES;
}

void PhysicalEnvironment::initialize(int stage) {
    switch (stage) {
        case inet::InitStages::INITSTAGE_LOCAL:
            initializePythonRuntime();
            break;
        case inet::InitStages::INITSTAGE_PHYSICAL_ENVIRONMENT:
            initializeScene();
            initializeSionnaAPI();
            break;
        default:
            break;
    }
}

void PhysicalEnvironment::finish() {
    scene_.reset();
    interpreter_.reset();
    omnetpp::cSimpleModule::finish();
}

void PhysicalEnvironment::initializePythonRuntime() {
    if (interpreter_) {
        return;
    }

    if (hasPar("virtualEnvironmentPath")) {
        interpreter_ = std::make_unique<ScopedInterpreter>(par("virtualEnvironmentPath").stdstringValue());
    } else {
        // clang-format off
        #if defined(SIONNA_VENV_HINT)
            interpreter_ = std::make_unique<ScopedInterpreter>(SIONNA_VENV_HINT);
        #else
            throw omnetpp::cRuntimeError("failed to initialize interpreter, python virtual environment is not set");
        #endif
        // clang-format on
    }

    nanobind::gil_scoped_acquire gil;
    auto mi = nanobind::module_::import_("mitsuba");

    // Variant is defined during compilation.
    using VariantName = mitsuba::detail::variant<mi::Float, mi::Spectrum>;
    mi.attr("set_variant")(VariantName::name);
}

void PhysicalEnvironment::initializeScene() {
    auto* provider = getSubmoduleAsType<IStaticSceneProvider>("sceneConfigProvider");
    scene_.emplace(provider->getSceneConfig());
}

void PhysicalEnvironment::initializeSionnaAPI() {
    dynamicConfiguration_ = std::make_shared<DynamicSceneConfigProxy>(this);
    IDConversion_ = std::make_shared<TraciIDConverterProxy>();

    auto traciNodeManagerPath = par("traciNodeManagerModule").stdstringValue();
    auto* traciNodeManagerModule = getModuleByPath(traciNodeManagerPath.c_str());
    auto* traciNodeManager = dynamic_cast<traci::BasicNodeManager*>(traciNodeManagerModule);

    if (traciNodeManager == nullptr) {
        throw omnetpp::cRuntimeError("No TraCI node manager found at path %s", traciNodeManagerPath.c_str());
    }

    auto transform = std::make_shared<AffineCoordinateTransform>(
        parseMatrix<mi::Matrix3f>(par("remapMatrix").xmlValue()),
        parseMatrix<mi::Matrix2f>(par("rotationMatrix").xmlValue()),
        parseTranslationVector(par("translationVector").xmlValue()),
        traci::Boundary(traciNodeManager->getAPI()->simulation.getNetBoundary()),
        this);

    if (auto roadMeshName = par("roadMeshName").stdstringValue(); !roadMeshName.empty()) {
        transform->meshResolver().addMesh(roadMeshName);
    }

    coordinateTransform_ = std::move(transform);
}

void PhysicalEnvironment::handleParameterChange(const char* /* parname */) {
}

void PhysicalEnvironment::refreshDisplay() const {
}

void PhysicalEnvironment::buildSceneFromEnvironment() {
}

void PhysicalEnvironment::updateDynamicObjects() {
}

inet::physicalenvironment::IObjectCache* PhysicalEnvironment::getObjectCache() const {
    return nullptr;
}

inet::physicalenvironment::IGround* PhysicalEnvironment::getGround() const {
    return nullptr;
}

const inet::Coord& PhysicalEnvironment::getSpaceMin() const {
    static const inet::Coord kZero;
    return kZero;
}

const inet::Coord& PhysicalEnvironment::getSpaceMax() const {
    static const inet::Coord kZero;
    return kZero;
}

const inet::physicalenvironment::IMaterialRegistry* PhysicalEnvironment::getMaterialRegistry() const {
    return &inet::physicalenvironment::MaterialRegistry::singleton;
}

int PhysicalEnvironment::getNumObjects() const {
    return 0;
}

const inet::physicalenvironment::IPhysicalObject* PhysicalEnvironment::getObject(int /* index */) const {
    return nullptr;
}

const inet::physicalenvironment::IPhysicalObject* PhysicalEnvironment::getObjectById(int /* id */) const {
    return nullptr;
}

void PhysicalEnvironment::visitObjects(
    const inet::IVisitor* visitor,
    const inet::LineSegment& lineSegment) const {
    if (!visitor) {
        return;
    }

    const int count = getNumObjects();
    for (int i = 0; i < count; ++i) {
        const auto* object = getObject(i);
        if (!object) {
            continue;
        }

        const auto* shape = object->getShape();
        if (!shape) {
            continue;
        }

        inet::Coord intersection1;
        inet::Coord intersection2;
        inet::Coord normal1;
        inet::Coord normal2;

        if (shape->computeIntersection(lineSegment, intersection1, intersection2, normal1, normal2)) {
            visitor->visit(dynamic_cast<const omnetpp::cObject*>(object));
        }
    }
}

const artery::sionna::py::SionnaScene& PhysicalEnvironment::scene() const {
    if (!scene_.has_value()) {
        throw omnetpp::cRuntimeError("physical environment scene is not initialized");
    }

    return scene_.value();
}

artery::sionna::py::SionnaScene& PhysicalEnvironment::scene() {
    if (!scene_.has_value()) {
        throw omnetpp::cRuntimeError("physical environment scene is not initialized");
    }

    return scene_.value();
}

mitsuba::ref<mi::Scene> PhysicalEnvironment::miScene() {
    return scene().miScene();
}

bool PhysicalEnvironment::setTxArray(const py::AntennaArray& array) {
    if (scene().txArray().has_value()) {
        return false;
    }

    scene().setTxArray(array);
    return true;
}

bool PhysicalEnvironment::setRxArray(const py::AntennaArray& array) {
    if (scene().rxArray().has_value()) {
        return false;
    }

    scene().setRxArray(array);
    return true;
}

IDynamicSceneConfigProxy* PhysicalEnvironment::dynamicConfiguration() {
    return dynamicConfiguration_.get();
}

ICoordinateTransformProxy* PhysicalEnvironment::coordinateTransform() {
    return coordinateTransform_.get();
}

IIDConverterProxy* PhysicalEnvironment::IDConversion() {
    return IDConversion_.get();
}
