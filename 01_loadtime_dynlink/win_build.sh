gcc -O2 -I/C/VulkanSDK/1.2.189.2/Include loadtime_dynlink.c -DVK_USE_PLATFORM_WIN32_KHR -o loadtime_dynlink.exe -Wl,--strip-all -L/C/VulkanSDK/1.2.189.2/Lib -lvulkan-1
