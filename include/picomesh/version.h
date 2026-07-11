#pragma once

#define PICOMESH_VERSION_MAJOR 0
#define PICOMESH_VERSION_MINOR 1
#define PICOMESH_VERSION_PATCH 0
#define PICOMESH_VERSION_STRING "0.1.0"

namespace picomesh {

inline constexpr unsigned version_major = PICOMESH_VERSION_MAJOR;
inline constexpr unsigned version_minor = PICOMESH_VERSION_MINOR;
inline constexpr unsigned version_patch = PICOMESH_VERSION_PATCH;
inline constexpr const char* version_string = PICOMESH_VERSION_STRING;

}  // namespace picomesh
