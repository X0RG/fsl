#ifndef FSL_H
#define FSL_H

#include <stdint.h>
#define SREF_NULL UINT32_MAX

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef const char* str;
typedef char* mut_str;

typedef enum {
  APPVEC_OK,
  APPVEC_ERR_NULL,   // case "NULL was passed"
  APPVEC_ERR_ALLOC,  // case "malloc | calloc | realloc failed"
} AppVecStatus;

typedef enum {
  APPLAUNCH_OK = 0,
  APPLAUNCH_ERR_NOT_FOUND,     // case "app url invalid"
  APPLAUNCH_ERR_OS,            // case "os error"
  APPLAUNCH_ERR_UNKNOWN = -1,  // case "_"
} AppLaunchStatus;

/* Offset into Arena::buf. */
typedef uint32_t sref;

/*
 * Arena : A contiguous byte buffer that owns all string storage.
 *
 * size     : bytes used
 * capacity : bytes allocated
 */
typedef struct Arena {
  mut_str buf;   // contiguous memory block
  u32 size;      // size used i.e number of bytes written
  u32 capacity;  // total bytes allocated i.e. written + unwritten
} Arena;

/* App : [4]{ id, name, icon, url }
 * Owns nothing. All fields are string references (offsets) into AppVec::arena.
 */
typedef struct App {
  sref id;
  sref name;
  sref icon;
  sref url;
} App;

/* AppVec : [4]{ data, arena, len, capacity }
 * Owns the App array and string arena.
 */
typedef struct AppVec {
  App* data;     // App array
  Arena arena;   // String arena
  u32 len;       // Number of active apps
  u32 capacity;  // Allocated App slots
} AppVec;

void fsl_api_init(void);
AppVecStatus fsl_scan_apps(str paths[], u32 paths_count, AppVec* out_v);
AppLaunchStatus fsl_launch_app(str id);

#endif