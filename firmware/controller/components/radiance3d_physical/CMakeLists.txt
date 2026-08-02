idf_component_register(
  SRCS
    "../../src/axis_controller.cpp"
    "../../src/hardware_config.cpp"
    "../../src/physical_motion_controller.cpp"
    "../../src/tmc2209_driver.cpp"
  INCLUDE_DIRS "../../include" "${RADIANCE3D_GENERATED_INCLUDE_DIR}"
  REQUIRES radiance3d_core platform_idf
)
