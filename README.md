# fsl — Fluid Shell Layer
fsl is an ultra-lightweight, embedded application-scanning and widget 
engine designed to bridge the UX gap between mobile/tablet layouts and 
traditional desktop environments. 

Instead of relying on heavy web-views (Electron, Tauri) or embedding 
massive runtimes, fsl takes an minimalist approach to systems engineering: 
A unified, bare-metal C engine that compiles down to a sub-100KB binary 
structure. This core exposes a pure C-ABI consumed directly by 100% 
native platform frontends (Swift/AppKit on macOS, C#/WinUI 3 on Windows, 
C++/GTK on Linux) to guarantee zero input lag and a microscopic memory 
footprint.

# Architecture & Performance Manifesto

1. Zero-Allocation String Arena (AppVec)
To avoid thousands of tiny heap allocations during filesystem scanning, 
fsl uses a specialized, contiguous byte buffer (Arena). 
* Individual App structs do not own their string fields (id, name, 
  icon, url).
* Instead, fields are stored as 32-bit unsigned offsets (sref) into 
  a single, dynamically growing text arena.
* Cache locality is tightly preserved, and freeing the entire scanned 
  payload requires exactly one free() call on the data array and one 
  on the arena buffer.

2. Microscopic Footprint
The shared business logic contains no garbage collection, no virtual 
machines, and no internal IPC sockets. Communication happens instantly 
at bare-metal CPU speeds across the standard C-ABI, keeping runtime 
overhead effectively at zero.

# Repository Structure

```bash
.
├── Makefile
├── README.txt
├── include/
│   └── fsl.h                 # Public C-ABI boundary interface & Types
└── src/
    ├── main.c                # Local development entry point & testing
    ├── core/
    │   ├── fsl_api.c         # Exposed API boundary hooks for frontends
    │   └── os_driver.h       # Internal cross-platform driver abstraction
    ├── interface/
    │   ├── mac_driver.c      # macOS native bundle parsing via CF / POSIX
    │   ├── win_driver.c      # Windows driver stub (Registry target)
    │   └── linux_driver.c    # Linux driver stub (.desktop target)
    └── utilities/
        ├── app_utils.c       # Vector growth and trimming mechanics
        └── app_utils.h       # Internal vector operation declarations
```

# Data Layout Specifications
The memory layouts are defined in the public include/fsl.h header for 
quick traversal and easy serialization across the FFI boundary:

```c
typedef uint32_t sref; // Offset into Arena::buf; SREF_NULL if empty```

typedef struct App {
    sref id;    // App unique identifier (Bundle ID / desktop filename)
    sref name;  // Display name
    sref icon;  // Resolution-independent icon path / resource URI
    sref url;   // Absolute executable or bundle entry path
} App;


typedef struct Arena {
    mut_str buf;       // Contiguous byte block containing raw strings
    uint32_t size;     // Current bytes consumed
    uint32_t capacity; // Total allocated bytes
} Arena;

typedef struct AppVec {
    App* data;         // Contiguous array of App instances
    Arena arena;       // Monotonic string storage arena
    uint32_t len;      // Active item count
    uint32_t capacity; // Pre-allocated slot capacity
} AppVec;
```
# Building and Benchmarking
Prerequisites:
* A standard C compiler (gcc or clang).
* make build utility.
* (macOS only) CoreFoundation framework.

Build from Source:
Compile optimized binaries utilizing link-time optimization (-flto) 
and architecture-native flag tuning:

    make

The compiled binary and object trees will be isolated under ./build/.

Current Benchmarks (macOS):
Tested via hyperfine on directory structures totaling 70+ applications 
(scanning deep path spaces like /Applications and /System/Applications 
recursively):

    hyperfine --runs 50 --warmup 5 -N './build/run'

* Time (mean ± σ):      16.0 ms ±   0.6 ms    [User: 8.4 ms, System: 6.3 ms]
* Range (min … max):    14.3 ms …  16.9 ms    50 runs

# Roadmap & Current Status
[X] Core Infrastructure: Memory arena mechanics, dynamic vector layout, 
    and automatic downward resizing vectors (app_vec_size_shrink).

[X] Flatter Directory Restructuring: Unified public types into fsl.h 
    and stripped away nested redundant header structures.

[X] macOS Driver (mac_driver.c): Recursive POSIX directory traversal 
    checking for .app bundle targets combined with low-level 
    CFBundleRef metadata extraction.

[ ] Linux Driver (linux_driver.c): Replace current stub implementation 
    to scan and parse standard .desktop files in /usr/share/applications/ 
    and ~/.local/share/applications/.

[ ] Windows Driver (win_driver.c): Implement low-level Windows Registry 
    walking using RegOpenKeyExA/RegEnumKeyExA against Uninstall hives.