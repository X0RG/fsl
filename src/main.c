#include <fsl.h>
#include <stdio.h>
#include <stdlib.h>
#include <utilities/app_utils.h>

int main() {
  fsl_api_init();

  str scan_targets[] = {
      "/Applications",
      "/System/Applications",
  };
  u32 targets_count = sizeof(scan_targets) / sizeof(scan_targets[0]);

  AppVec my_apps;
  app_vec_init(&my_apps);

  fsl_scan_apps(scan_targets, targets_count, &my_apps);

  for (u32 i = 0; i < my_apps.len; ++i) {
    u32 id = my_apps.data[i].id;
    str id_str = my_apps.arena.buf + id;
    printf("%u - %u - %s\n", i, id, id_str);
  }
  app_vec_free(&my_apps);

  return 0;
}