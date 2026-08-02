#pragma once

namespace radiance3d {

// Startup is split from app_main so reset/NVS initialization remains explicit
// and task creation can be tested independently of global constructors.
bool controller_runtime_initialize(const char* reset_reason);
void controller_runtime_start();

}  // namespace radiance3d
