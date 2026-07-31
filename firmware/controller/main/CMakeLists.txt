idf_component_register(
  SRCS "app_main.cpp" "controller_runtime.cpp"
  INCLUDE_DIRS "." "../include" "${RADIANCE3D_GENERATED_INCLUDE_DIR}"
  REQUIRES radiance3d_core radiance3d_physical platform_idf
  PRIV_REQUIRES nvs_flash esp_system esp_timer driver freertos log
)
