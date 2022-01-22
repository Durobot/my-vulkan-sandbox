gcc -O2 -I/C/VulkanSDK/1.2.189.2/Include -o runtime_dynlink.exe -Wl,--strip-all  -DVK_USE_PLATFORM_WIN32_KHR runtime_dynlink.c
