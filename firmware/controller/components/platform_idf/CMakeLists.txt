idf_component_register(
  SRCS "src/idf_hardware_platform.cpp" "src/idf_step_pulse_scheduler.cpp"
  INCLUDE_DIRS "include" "../../include"
  REQUIRES driver esp_timer freertos
)
