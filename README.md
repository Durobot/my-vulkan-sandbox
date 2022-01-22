# my-vulkan-sandbox
Just a box of short programs using Vulkan API, created for self-teaching / as a cheatsheet.

Contents:

**01_loadtime_dynlink** - demonstrates the easiest way of using Vulkan loader library (vulkan-1.dll on Windows, libvulkan.so on Linux, .dylib on macOS), linking against the library at build time. Simple, but not the best performance (see comments in runtime_dynlink.c).

**02_runtime_dynlink** - demonstrates a more advanced way of using Vulkan loader library (vulkan-1.dll on Windows, libvulkan.so on Linux, .dylib on macOS), loading the library at runtime by using OS functions. See comments in runtime_dynlink.c for reasons why this is better than linking against this library at build time.
