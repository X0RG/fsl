#include <fsl.h>
#include <stdio.h>
#include <stdlib.h>
#include <utilities/app_utils.h>

int main() {
  fsl_api_init();

  str scan_targets[] = {
      "/usr/share/applications",
      "/usr/local/share/applications",
      "~/.local/share/applications",
      "/var/lib/flatpak/exports/share/applications",
      "~/.local/share/flatpak/exports/share/applications",
      "/var/lib/snapd/desktop/applications",
      "~/.local/bin",
      "~/Applications",
  };
  u32 targets_count = sizeof(scan_targets) / sizeof(scan_targets[0]);

  AppVec my_apps;
  app_vec_init(&my_apps);

  fsl_scan_apps(scan_targets, targets_count, &my_apps);
  for (u32 i = 0; i < my_apps.len; ++i) {
    str id = my_apps.arena.buf + my_apps.data[i].id;
    str url = my_apps.arena.buf + my_apps.data[i].url;
    fsl_launch_app(id, url);
  }
  app_vec_print(&my_apps);
  app_vec_free(&my_apps);

  return 0;
}