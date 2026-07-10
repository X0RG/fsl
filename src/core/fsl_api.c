#include <core/os_driver.h>

static OsDriver global_driver;

void fsl_api_init(void) {
#if defined(__APPLE__)
  global_driver = mac_driver_init();
#elif defined(_WIN32)
  global_driver = win_driver_init();
#elif defined(__linux__)
  global_driver = linux_driver_init();
#else
#error "Unsupported Operating System!"
#endif
}

AppVecStatus fsl_scan_apps(str paths[], u32 paths_count, AppVec* out_v) {
  return global_driver.scan_apps(paths, paths_count, out_v);
}

AppLaunchStatus fsl_launch_app(str id, str url) {
  return global_driver.launch_app(id, url);
}