#pragma once

/** PicoMesh semantic-version major component. */
#define PICOMESH_VERSION_MAJOR 0
/** PicoMesh semantic-version minor component. */
#define PICOMESH_VERSION_MINOR 1
/** PicoMesh semantic-version patch component. */
#define PICOMESH_VERSION_PATCH 0
/** PicoMesh semantic version as a string literal. */
#define PICOMESH_VERSION_STRING "0.1.0"

namespace picomesh {

/** Compile-time semantic-version major component. */
inline constexpr unsigned version_major = PICOMESH_VERSION_MAJOR;
/** Compile-time semantic-version minor component. */
inline constexpr unsigned version_minor = PICOMESH_VERSION_MINOR;
/** Compile-time semantic-version patch component. */
inline constexpr unsigned version_patch = PICOMESH_VERSION_PATCH;
/** Compile-time semantic version string. */
inline constexpr const char* version_string = PICOMESH_VERSION_STRING;

} // namespace picomesh
