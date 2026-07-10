#include <core/os_driver.h>
#include <stdio.h>
#include <stdlib.h>

static AppVecStatus linux_scan_applications(str paths[], u32 paths_count,
                                            AppVec* out_v) {
  // TODO: Parse /usr/share/applications/*.desktop files

  (void)paths;
  (void)paths_count;
  (void)out_v;
  printf("[LINUX] Scanning applications...\n");
  return APPVEC_OK;
}

static AppLaunchStatus linux_launch_application(str id) {
  // TODO: fork() and execvp() the desktop shortcut target

  (void)id;
  printf("[LINUX] Launching application with ID: %s\n", id);
  return APPLAUNCH_OK;
}

OsDriver linux_driver_init(void) {
  return (OsDriver){
      .scan_apps = linux_scan_applications,
      .launch_app = linux_launch_application,
  };
}