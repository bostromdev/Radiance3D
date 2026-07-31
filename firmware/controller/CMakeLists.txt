cmake_minimum_required(VERSION 3.16)

# ESP-IDF 5.5 uses generated response files during compiler detection.  Keep
# CMake's normal toolchain-file variable when its cache entry is created so
# external ESP-IDF subprojects (including the bootloader) inherit it reliably.
if(POLICY CMP0126)
  # project.cmake declares its own 3.16 baseline, so setting the policy in
  # this scope alone is not enough.  The default is inherited by ESP-IDF's
  # toolchain and bootloader project scopes as they are created.
  set(CMAKE_POLICY_DEFAULT_CMP0126 NEW)
  cmake_policy(SET CMP0126 NEW)
endif()

# This is intentionally the source of truth for the physical ESP32 target.
# Host builds live in host/CMakeLists.txt and do not require ESP-IDF.
find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(RADIANCE3D_PROFILE
    "${CMAKE_CURRENT_LIST_DIR}/../config/provisional-esp32dev-v1.json")
set(RADIANCE3D_GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${RADIANCE3D_GENERATED_INCLUDE_DIR}")
set(RADIANCE3D_GENERATED_PROFILE_HEADER
    "${RADIANCE3D_GENERATED_INCLUDE_DIR}/hardware_profile_generated.hpp")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${RADIANCE3D_PROFILE}"
  "${CMAKE_CURRENT_LIST_DIR}/../../scripts/generate_hardware_profile_header.py")

execute_process(
  COMMAND "${Python3_EXECUTABLE}"
          "${CMAKE_CURRENT_LIST_DIR}/../../scripts/generate_hardware_profile_header.py"
          --profile "${RADIANCE3D_PROFILE}"
          --output "${RADIANCE3D_GENERATED_PROFILE_HEADER}"
  RESULT_VARIABLE RADIANCE3D_PROFILE_RESULT
  OUTPUT_VARIABLE RADIANCE3D_PROFILE_OUTPUT
  ERROR_VARIABLE RADIANCE3D_PROFILE_ERROR
)
if(NOT RADIANCE3D_PROFILE_RESULT EQUAL 0)
  message(FATAL_ERROR "Could not generate hardware profile header: ${RADIANCE3D_PROFILE_ERROR}")
endif()

set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/components")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# The bootloader is configured as an external ESP-IDF subproject.  Forward the
# policy default so it uses the same response-file handling on newer CMake.
if(POLICY CMP0126)
  idf_build_set_property(
    EXTRA_CMAKE_ARGS "-DCMAKE_POLICY_DEFAULT_CMP0126=NEW" APPEND)
endif()
project(radiance3d_controller)
