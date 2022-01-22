gcc -O2  -o runtime_dynlink -Wl,--strip-all  -DVK_USE_PLATFORM_XCB_KHR runtime_dynlink.c
