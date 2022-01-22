gcc -O2  loadtime_dynlink.c -DVK_USE_PLATFORM_XCB_KHR -o loadtime_dynlink -Wl,--strip-all  -lvulkan
