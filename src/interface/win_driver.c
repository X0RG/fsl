#include <core/os_driver.h>
#include <stdio.h>
#include <stdlib.h>

static AppVecStatus win_scan_applications(str paths[], u32 paths_count,
                                          AppVec* out_v) {
  // TODO: Query Windows Registry for installed software

  (void)paths;
  (void)paths_count;
  (void)out_v;
  printf("[Windows] Scanning applications...\n");
  return APPVEC_OK;
}

static AppLaunchStatus win_launch_application(str id, str url) {
  // TODO: Call ShellExecuteA or CreateProcess

  (void)id;
  (void)url;
  printf("[Windows] Launching application with ID: %s\n", id);
  return APPLAUNCH_OK;
}

OsDriver win_driver_init(void) {
  return (OsDriver){
      .scan_apps = win_scan_applications,
      .launch_app = win_launch_application,
  };
}