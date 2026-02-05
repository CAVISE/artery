#include <gtest/gtest.h>

#include <artery/sionna/bridge/Compat.h>
#include <artery/sionna/bridge/Fwd.h>

#include <drjit/array.h>
#include <drjit/jit.h>

#include <tuple>

using Float = drjit::LLVMArray<float>;
using Spectrum = mitsuba::Color<Float, 3>;

SIONNA_IMPORT_CORE_TYPES(Point3f, Vector3f, BoundingBox3f)
SIONNA_IMPORT_BRIDGE_TYPES(Compat)

TEST(CompatLLVM, ToScalarExtractsFirstLane) {
    Float values = drjit::arange<Float>(4);

    const double scalar = Compat::toScalar(values);
    EXPECT_DOUBLE_EQ(scalar, 0.0);
}

TEST(CompatLLVM, FromScalarProducesLLVMArray) {
    const Float produced = Compat::fromScalar(4.5);

    drjit::eval(produced);
    ASSERT_EQ(drjit::width(produced), 1);
    EXPECT_FLOAT_EQ(Compat::toScalar<float>(produced), 4.5f);
}

TEST(CompatLLVM, PointToCoordFiniteValues) {
    Point3f point(Float(1.0f), Float(-2.0f), Float(3.5f));
    const inet::Coord coord = Compat::toCoord(point);

    EXPECT_DOUBLE_EQ(coord.x, 1.0);
    EXPECT_DOUBLE_EQ(coord.y, -2.0);
    EXPECT_DOUBLE_EQ(coord.z, 3.5);
}

TEST(CompatLLVM, PointToCoordRejectsNaN) {
    Point3f point(Float(0.0f), Float(0.0f), Float(0.0f));

    const inet::Coord coord = Compat::toCoord(point);
    EXPECT_EQ(coord, inet::Coord::NIL);
}

TEST(CompatLLVM, BoundingBoxExtentsToCoord) {
    Point3f min(Float(0.0f), Float(0.0f), Float(1.0f));
    Point3f max(Float(1.0f), Float(2.0f), Float(4.5f));
    BoundingBox3f box(min, max);

    const inet::Coord coord = Compat::toCoord(box);

    EXPECT_DOUBLE_EQ(coord.x, 1.0);
    EXPECT_DOUBLE_EQ(coord.y, 2.0);
    EXPECT_DOUBLE_EQ(coord.z, 3.5);
}

TEST(CompatLLVM, InvalidBoundingBoxReturnsNil) {
    BoundingBox3f invalid;

    const inet::Coord coord = Compat::toCoord(invalid);
    EXPECT_EQ(coord, inet::Coord::NIL);
}

TEST(CompatLLVM, VectorToEulerAngles) {
    Vector3f vector(Float(0.1f), Float(-0.2f), Float(0.3f));
    const inet::EulerAngles angles = Compat::toEuler(vector);

    EXPECT_DOUBLE_EQ(angles.alpha, 0.1);
    EXPECT_DOUBLE_EQ(angles.beta, -0.2);
    EXPECT_DOUBLE_EQ(angles.gamma, 0.3);
}

TEST(CompatLLVM, TupleToFigureColorClamps) {
    const auto color = Compat::toColor(
        std::make_tuple(1.1f, -0.2f, 0.5f)
    );

    EXPECT_EQ(color.red, 255);
    EXPECT_EQ(color.green, 0);
    EXPECT_EQ(color.blue, 128);
}
