#define _DEFAULT_SOURCE
#include <core/os_driver.h>
#include <dirent.h>
#include <fcntl.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utilities/app_utils.h>

extern char** environ;

#define DESKTOP_BUFFER_SIZE 4096

static inline void sanitize_value(char* val) {
  char* end = val;
  while (*end && *end != '\n' && *end != '\r') {
    end++;
  }
  *end = '\0';
}

static void parse_desktop_file(const char* filepath, const char* filename,
                               AppVec* out_v) {
  int fd = open(filepath, O_RDONLY);
  if (fd < 0) return;

  char buf[DESKTOP_BUFFER_SIZE];
  ssize_t bytes_read = read(fd, buf, sizeof(buf) - 1);
  close(fd);

  if (bytes_read <= 0) return;
  buf[bytes_read] = '\0';

  char* section = strstr(buf, "[Desktop Entry]");
  if (!section) return;

  char* line = section;
  char* name = NULL;
  char* exec = NULL;
  char* icon = NULL;
  int no_display = 0;

  while ((line = strchr(line, '\n')) != NULL) {
    line++;
    if (*line == '[') break;
    if (*line == '\0') break;

    if (strncmp(line, "Name=", 5) == 0) {
      name = line + 5;
    } else if (strncmp(line, "Exec=", 5) == 0) {
      exec = line + 5;
    } else if (strncmp(line, "Icon=", 5) == 0) {
      icon = line + 5;
    } else if (strncmp(line, "NoDisplay=true", 14) == 0) {
      no_display = 1;
      break;
    }
  }

  if (no_display || !name || !exec) return;

  sanitize_value(name);
  sanitize_value(exec);
  if (icon) {
    sanitize_value(icon);
  } else {
    icon = "system-run";
  }

  char* p = exec;
  while (*p) {
    if (*p == ' ' && *(p + 1) == '%') {
      *p = '\0';
      break;
    }
    p++;
  }

  app_vec_item_insert(out_v, (str)filename, (str)name, (str)icon, (str)exec);
}

static void scan_desktop_dir(const char* dir_path, AppVec* out_v) {
  DIR* dir = opendir(dir_path);
  if (!dir) return;

  struct dirent* entry;
  char path_buf[1024];

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG || entry->d_type == DT_LNK ||
        entry->d_type == DT_UNKNOWN) {
      size_t len = strlen(entry->d_name);
      if (len > 8 && strcmp(entry->d_name + len - 8, ".desktop") == 0) {
        snprintf(path_buf, sizeof(path_buf), "%s/%s", dir_path, entry->d_name);
        parse_desktop_file(path_buf, entry->d_name, out_v);
      }
    }
  }
  closedir(dir);
}

static AppVecStatus linux_scan_applications(str paths[], u32 paths_count,
                                            AppVec* out_v) {
  if (!out_v) return APPVEC_ERR_NULL;

  app_vec_reset(out_v);

  if (!paths || paths_count == 0) {
    scan_desktop_dir("/usr/share/applications", out_v);

    char home_dir[512];
    const char* xdg_data = getenv("XDG_DATA_HOME");
    if (xdg_data && xdg_data[0] != '\0') {
      snprintf(home_dir, sizeof(home_dir), "%s/applications", xdg_data);
    } else {
      const char* home = getenv("HOME");
      if (home) {
        snprintf(home_dir, sizeof(home_dir), "%s/.local/share/applications",
                 home);
      } else {
        home_dir[0] = '\0';
      }
    }
    if (home_dir[0] != '\0') {
      scan_desktop_dir(home_dir, out_v);
    }
  } else {
    for (u32 i = 0; i < paths_count; ++i) {
      if (paths[i]) scan_desktop_dir(paths[i], out_v);
    }
  }

  return APPVEC_OK;
}

static AppLaunchStatus linux_launch_application(str id, str url) {
  (void)id;
  str target_exec = url;
  if (!target_exec || target_exec[0] == '\0') return APPLAUNCH_ERR_NOT_FOUND;

  pid_t pid;
  char exec_copy[512];
  strncpy(exec_copy, target_exec, sizeof(exec_copy) - 1);
  exec_copy[sizeof(exec_copy) - 1] = '\0';

  char* argv[32];
  int argc = 0;
  char* token = strtok(exec_copy, " ");
  while (token && argc < 31) {
    argv[argc++] = token;
    token = strtok(NULL, " ");
  }
  argv[argc] = NULL;

  if (argc == 0) return APPLAUNCH_ERR_NOT_FOUND;

  int status = posix_spawn(&pid, argv[0], NULL, NULL, argv, environ);
  if (status != 0) {
    return APPLAUNCH_ERR_OS;
  }

  int exit_status;
  waitpid(pid, &exit_status, WNOHANG);

  return APPLAUNCH_OK;
}

OsDriver linux_driver_init(void) {
  return (OsDriver){
      .scan_apps = linux_scan_applications,
      .launch_app = linux_launch_application,
  };
}