#pragma once

#include <cavise/sionna/bridge/bindings/Scene.h>

#include <nanobind/nanobind.h>

#include <string>

namespace artery::sionna {

    /**
     * @brief Encapsulates a Python callback function for custom scene rendering.
     *
     * This class handles resolution of Python renderer specifications in the format
     * "path/to/file.py:function" or "module:function" and provides a method
     * to invoke the callback with the necessary parameters.
     */
    class PythonRendererCallback {
    public:
        PythonRendererCallback() = default;

        /**
         * @brief Resolve a Python renderer from a specification string.
         *
         * @param rendererSpec Specification in format "path/to/file.py:function"
         *                    or "module:function"
         * @return true if resolution was successful
         * @return false if spec is empty or resolution failed
         */
        bool resolve(const std::string& rendererSpec);

        /**
         * @brief Invoke the Python callback with the given parameters.
         *
         * @param scene The SionnaScene object to pass to the callback
         * @param simTime Current simulation time
         * @param outputDir Output directory path
         * @param frameIndex Current frame index
         * @throws omnetpp::cRuntimeError if callback invocation fails
         */
        void invoke(py::SionnaScene scene, double simTime, const std::string& outputDir, int frameIndex);

        /**
         * @brief Check if a valid callback has been resolved.
         *
         * @return true if callback is valid and can be invoked
         * @return false otherwise
         */
        bool isValid() const;

    private:
        nanobind::object callback_;
    };

} // namespace artery::sionna