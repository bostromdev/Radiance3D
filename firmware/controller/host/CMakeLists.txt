cmake_minimum_required(VERSION 3.20)
project(radiance3d_controller_host LANGUAGES CXX)

include(CTest)
find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(RADIANCE3D_CONTROLLER_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
set(RADIANCE3D_REPOSITORY_DIR "${RADIANCE3D_CONTROLLER_DIR}/../..")
set(RADIANCE3D_GENERATED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${RADIANCE3D_GENERATED_INCLUDE_DIR}")
set(RADIANCE3D_GENERATED_PROFILE_HEADER
    "${RADIANCE3D_GENERATED_INCLUDE_DIR}/hardware_profile_generated.hpp")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${RADIANCE3D_CONTROLLER_DIR}/../config/provisional-esp32dev-v1.json"
  "${RADIANCE3D_REPOSITORY_DIR}/scripts/generate_hardware_profile_header.py")

execute_process(
  COMMAND "${Python3_EXECUTABLE}"
          "${RADIANCE3D_REPOSITORY_DIR}/scripts/generate_hardware_profile_header.py"
          --profile "${RADIANCE3D_CONTROLLER_DIR}/../config/provisional-esp32dev-v1.json"
          --output "${RADIANCE3D_GENERATED_PROFILE_HEADER}"
  RESULT_VARIABLE RADIANCE3D_PROFILE_RESULT
  ERROR_VARIABLE RADIANCE3D_PROFILE_ERROR
)
if(NOT RADIANCE3D_PROFILE_RESULT EQUAL 0)
  message(FATAL_ERROR "Could not generate hardware profile header: ${RADIANCE3D_PROFILE_ERROR}")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Some Command Line Tools installations keep libc++ only in the active SDK.
# Add it as a system include when present; normal macOS and non-macOS builds
# keep their compiler-default standard-library search paths unchanged.
set(RADIANCE3D_HOST_SDK_CXX_INCLUDE "")
if(APPLE)
  execute_process(
    COMMAND xcrun --show-sdk-path
    OUTPUT_VARIABLE RADIANCE3D_HOST_SDK_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  if(EXISTS "${RADIANCE3D_HOST_SDK_PATH}/usr/include/c++/v1/cstdint")
    set(RADIANCE3D_HOST_SDK_CXX_INCLUDE
        "${RADIANCE3D_HOST_SDK_PATH}/usr/include/c++/v1")
  endif()
endif()

add_library(radiance3d_core
  "${RADIANCE3D_CONTROLLER_DIR}/src/motion_controller.cpp"
  "${RADIANCE3D_CONTROLLER_DIR}/src/protocol.cpp"
)
target_include_directories(radiance3d_core PUBLIC
  "${RADIANCE3D_CONTROLLER_DIR}/include"
  "${RADIANCE3D_GENERATED_INCLUDE_DIR}"
)
if(RADIANCE3D_HOST_SDK_CXX_INCLUDE)
  target_include_directories(radiance3d_core SYSTEM BEFORE PUBLIC
    "${RADIANCE3D_HOST_SDK_CXX_INCLUDE}"
  )
endif()

add_library(radiance3d_physical
  "${RADIANCE3D_CONTROLLER_DIR}/src/axis_controller.cpp"
  "${RADIANCE3D_CONTROLLER_DIR}/src/hardware_config.cpp"
  "${RADIANCE3D_CONTROLLER_DIR}/src/physical_motion_controller.cpp"
  "${RADIANCE3D_CONTROLLER_DIR}/src/tmc2209_driver.cpp"
)
target_link_libraries(radiance3d_physical PUBLIC radiance3d_core)
target_include_directories(radiance3d_physical PUBLIC
  "${RADIANCE3D_CONTROLLER_DIR}/include"
  "${RADIANCE3D_GENERATED_INCLUDE_DIR}"
)

add_executable(radiance3d-simulator simulator_main.cpp)
target_link_libraries(radiance3d-simulator PRIVATE radiance3d_core)

add_library(radiance3d_host_test INTERFACE)
target_include_directories(radiance3d_host_test INTERFACE
  "${CMAKE_CURRENT_LIST_DIR}/test_support"
)

function(add_radiance3d_test name source)
  add_executable(${name} "${source}")
  target_link_libraries(${name} PRIVATE radiance3d_physical radiance3d_host_test)
  add_test(NAME ${name} COMMAND ${name})
endfunction()

add_radiance3d_test(test_axis "${RADIANCE3D_CONTROLLER_DIR}/test/test_axis/test_main.cpp")
add_radiance3d_test(test_driver "${RADIANCE3D_CONTROLLER_DIR}/test/test_driver/test_main.cpp")
add_radiance3d_test(test_motion "${RADIANCE3D_CONTROLLER_DIR}/test/test_motion/test_main.cpp")
add_radiance3d_test(test_physical "${RADIANCE3D_CONTROLLER_DIR}/test/test_physical/test_main.cpp")
