// #include <memory>
// #include <fstream>
// #include <optional>
// #include <filesystem>

// #include <gtest/gtest.h>
// #include <nanobind/nanobind.h>

// #include <mitsuba/core/fwd.h>
// #include <mitsuba/core/ray.h>
// #include <mitsuba/core/plugin.h>
// #include <mitsuba/core/config.h>
// #include <mitsuba/core/transform.h>
// #include <mitsuba/core/properties.h>

// #include <mitsuba/render/film.h>
// #include <mitsuba/render/scene.h>
// #include <mitsuba/render/shape.h>
// #include <mitsuba/render/sensor.h>
// #include <mitsuba/render/integrator.h>

// #include <artery/sionna/bridge/Meshes.h>
// #include <artery/sionna/bridge/Helpers.h>

// using Float = float;
// using Spectrum = mitsuba::Color<Float, 3>;

// namespace nb = nanobind;
// namespace fs = std::filesystem;

// SIONNA_IMPORT_CORE_TYPES(Vector3d, Vector3f, AffineTransform4f, Point3f)
// SIONNA_IMPORT_RENDER_TYPES(Film, Sensor, Integrator, Shape, Scene)

// namespace {

//     class SionnaLightTest
//         : public ::testing::Test {
//     protected:

//         void SetUp() override {
//             mi = std::make_unique<nb::module_>(nb::module_::import_("mitsuba"));
//             mi->attr("set_variant")("scalar_rgb");
//             pluginManager = mitsuba::PluginManager::instance();
//         }

//         mitsuba::Properties getFilmConfig() {
//             mitsuba::Properties props ("hdrfilm");
//             props.set("width", 1920);
//             props.set("height", 1080);
//             return props;
//         }

//         mitsuba::Properties getSensorConfig(std::optional<mitsuba::Properties> film = std::nullopt) {
//             mitsuba::Properties props ("perspective");
//             props.set("fov", 45);
//             props.set("to_world", AffineTransform4f::look_at(
//                 Point3f(0.0f, 0.0f, 6.0f),
//                 Point3f(0.0f, 0.0f, 0.0f),
//                 Point3f(0.0f, 1.0f, 0.0f)
//             ));

//             auto obj = pluginManager->create_object<Film>(film.value_or(getFilmConfig()));
//             props.set("film", nb::ref<mitsuba::Object>(obj));
//             return props;
//         }

//         mitsuba::Properties getIntegratorConfig() {
//             mitsuba::Properties props ("path");
//             return props;
//         }

//         mitsuba::Properties getShapeConfig(std::string filename) {
//             mitsuba::Properties props ("obj");
//             // props.set("filename", filename.c_str());
//             return props;
//         }

//         mitsuba::PluginManager* pluginManager;
//         std::unique_ptr<nb::module_> mi;
//     };

// }

// TEST_F(SionnaLightTest, GeneratedCubeObjLoadableByMitsuba) {
//     Vector3d sizes(2.0, 2.0, 2.0);

//     fs::path path = fs::temp_directory_path() / "cube.obj";
//     {
//         std::ofstream os (path);
//         ASSERT_TRUE(os.good()) << "Failed to open temporary OBJ file for writing: " << path.string();
//         artery::sionna::Meshes<Float, Spectrum>::createCubeObject(os, sizes);
//     }

//     mitsuba::Properties props ("scene");
//     auto sensor = pluginManager->create_object<Sensor>(getSensorConfig());
//     auto integrator = pluginManager->create_object<Integrator>(getIntegratorConfig());
//     // auto shape = pluginManager->create_object<Shape>(getShapeConfig(path.string()));

//     props.set("sensor", nb::ref<mitsuba::Object>(sensor));
//     props.set("integrator", nb::ref<mitsuba::Object>(integrator));
//     // props.set("shape", nb::ref<mitsuba::Object>(shape));

//     auto scene = pluginManager->create_object<Scene>(props);

//     ASSERT_NO_FATAL_FAILURE(
//         scene->integrator()->render(scene, static_cast<std::uint32_t>(0));
//     ) << "Mitsuba failed to render sample scene.";
//     // ASSERT_EQ(scene->shapes().size(), 1) << "Scene should contain exactly one shape";

//     // ASSERT_NO_FATAL_FAILURE(
//     //     sensor->film()->write(mitsuba::filesystem::path(path));
//     // ) << "Mitsuba failed to write rendered image to a file.";
// }
