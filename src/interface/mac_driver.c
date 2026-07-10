#include <CoreFoundation/CoreFoundation.h>
#include <core/os_driver.h>
#include <dirent.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utilities/app_utils.h>

extern char** environ;

static str bundle_identifier(str url) {
  if (!url || url[0] == '\0') return NULL;

  char* result = NULL;
  CFStringRef path_str =
      CFStringCreateWithFileSystemRepresentation(kCFAllocatorDefault, url);
  if (!path_str) return NULL;

  CFURLRef app_url = CFURLCreateWithFileSystemPath(
      kCFAllocatorDefault, path_str, kCFURLPOSIXPathStyle, true);
  CFRelease(path_str);
  if (!app_url) return NULL;

  CFBundleRef bundle = CFBundleCreate(kCFAllocatorDefault, app_url);
  CFRelease(app_url);
  if (!bundle) return NULL;

  CFStringRef bundle_id = CFBundleGetIdentifier(bundle);
  if (bundle_id) {
    CFIndex length = CFStringGetMaximumSizeForEncoding(
                         CFStringGetLength(bundle_id), kCFStringEncodingUTF8) +
                     1;
    result = malloc(length);
    if (result) {
      if (!CFStringGetCString(bundle_id, result, length,
                              kCFStringEncodingUTF8)) {
        free(result);
        result = NULL;
      }
    }
  }

  CFRelease(bundle);
  return (str)result;
}

static void scan_dir_recursive(str target_dir, u32* id_counter, AppVec* out_v) {
  DIR* dir = opendir(target_dir);
  if (!dir) return;

  struct dirent* entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 ||
        strcmp(entry->d_name, "Library") == 0 ||
        strcmp(entry->d_name, "Frameworks") == 0 ||
        strcmp(entry->d_name, "private") == 0 ||
        strcmp(entry->d_name, "usr") == 0 ||
        strcmp(entry->d_name, "var") == 0 ||
        strcmp(entry->d_name, "dev") == 0) {
      continue;
    }

    size_t len = strlen(entry->d_name);

    char full_path[1024];
    if (strcmp(target_dir, "/") == 0) {
      snprintf(full_path, sizeof(full_path), "/%s", entry->d_name);
    } else {
      snprintf(full_path, sizeof(full_path), "%s/%s", target_dir,
               entry->d_name);
    }

    if (len > 4 && strcmp(entry->d_name + len - 4, ".app") == 0) {
      char name_buf[256];
      size_t name_len = len - 4 > 255 ? 255 : len - 4;
      strncpy(name_buf, entry->d_name, name_len);
      name_buf[name_len] = '\0';

      str mock_icon = "/img/generic_app.png";
      str bundle_id = bundle_identifier(full_path);

      if (bundle_id) {
        app_vec_item_insert(out_v, bundle_id, name_buf, mock_icon, full_path);
        free((void*)bundle_id);
      } else {
        app_vec_item_insert(out_v, "unknown", name_buf, mock_icon, full_path);
      }
      continue;
    }

    if (entry->d_type == DT_DIR) {
      scan_dir_recursive(full_path, id_counter, out_v);
    }
  }
  closedir(dir);
}

static AppVecStatus mac_scan_apps(str paths[], u32 paths_count, AppVec* out_v) {
  if (!out_v || !paths) return APPVEC_ERR_NULL;

  app_vec_reset(out_v);
  u32 id_counter = 1;

  for (u32 p = 0; p < paths_count; ++p) {
    str target_dir = paths[p];
    if (!target_dir) continue;

    scan_dir_recursive(target_dir, &id_counter, out_v);
  }

  return APPVEC_OK;
}

static AppLaunchStatus mac_launch_app(str id, str url) {
  if (!id || id[0] == '\0') return APPLAUNCH_ERR_NOT_FOUND;
  (void)url;
  pid_t pid;
  char* argv[] = {"/usr/bin/open", "-b", (mut_str)id, NULL};

  int status = posix_spawn(&pid, "/usr/bin/open", NULL, NULL, argv, environ);
  if (status != 0) {
    return APPLAUNCH_ERR_OS;
  }

  int exit_status;
  waitpid(pid, &exit_status, 0);

  if (WIFEXITED(exit_status) && WEXITSTATUS(exit_status) == 0) {
    return APPLAUNCH_OK;
  }

  return APPLAUNCH_ERR_OS;
}

OsDriver mac_driver_init(void) {
  return (OsDriver){
      .scan_apps = mac_scan_apps,
      .launch_app = mac_launch_app,
  };
}