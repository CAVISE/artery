#pragma once

#include <artery/sionna/bridge/Helpers.h>
#include <nanobind/nanobind.h>

#include <cstdio>

/**
 * @brief This file defines Sionna project layout with
 * module and class names. It also provides functional-style
 * accessors to known python modules.
 */

NAMESPACE_BEGIN(artery)
NAMESPACE_BEGIN(sionna)

/**
 * @brief Sionna python module layout.
 */
class ModuleLayout
{
public:
    // Modules.
    static const std::string sionna;
    static const std::string sionnaRt;
    static const std::string sionnaConstants;

    // Known classes.
    class Classes
    {
    public:
        static const std::string IntersectionTypes;
        static const std::string radioMaterial;
    };
};

/**
 * @brief Access modules by factory functions.
 */
class Access
{
public:
    static nanobind::module_ sionna() { return doImport(ModuleLayout::sionna); }
    static nanobind::module_ sionnaRt() { return doImport(ModuleLayout::sionnaRt); }

    static nanobind::object getClass(nanobind::module_ mod, const std::string& cls) { return mod.attr(cls.c_str()); }

private:
    static nanobind::module_ doImport(const std::string& mod) { return nanobind::module_::import_(mod.c_str()); }
};

NAMESPACE_END(sionna)
NAMESPACE_END(artery)
