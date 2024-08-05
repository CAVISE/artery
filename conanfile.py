from conan import ConanFile
from conan.tools.cmake import cmake_layout

class Artery(ConanFile):
    version = "0.0"
    generators = ["CMakeToolchain", "CMakeDeps"]
    settings = "os", "compiler", "build_type", "arch"
    tool_requires = ["protobuf/3.18.1"]

    def requirements(self):
        for req in [
            "boost/1.71.0",
            "cryptopp/8.2.0",
            "protobuf/3.18.1",
            "geographiclib/2.3",
            "zeromq/4.3.2"
        ]:
            self.requires(req)

    def layout(self):
        cmake_layout(self)
