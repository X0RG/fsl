#ifndef OS_DRIVER_H
#define OS_DRIVER_H

#include <fsl.h>

typedef struct OsDriver {
  AppVecStatus (*scan_apps)(str paths[], u32 paths_count, AppVec* out_v);
  AppLaunchStatus (*launch_app)(str id);
} OsDriver;

OsDriver mac_driver_init(void);
OsDriver win_driver_init(void);
OsDriver linux_driver_init(void);

#endif