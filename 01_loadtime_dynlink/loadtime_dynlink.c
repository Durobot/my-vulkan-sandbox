// Load-time dynamic linking with Vulkan loader dynamic library (vulkan-1.dll on Windows, libvulkan.so on Linux) example.
// This method is referred to as "Vulkan Direct Exports / Dynamic Linking" below.
//
// Load-time dynamic linking creates a "baked in" connection between the executable and the library, unlike
// run-time dynamic linking, which amounts to calling OS functions to load the Vulkan loader library, get address of Vulkan loader's
// vkGetInstanceProcAddr() function, then calling it to get addresses of other Vulkan functions.
// With load-time dynamic linking Vulkan functions are available without this procedure.
// 
// https://github.com/KhronosGroup/Vulkan-Loader/blob/master/docs/LoaderApplicationInterface.md#directly-linking-to-the-loader
//
// From
// https://github.com/KhronosGroup/Vulkan-Loader/blob/master/docs/LoaderInterfaceArchitecture.md :
// The Loader (vulkan-1.dll / libvulkan.so)
// The application sits at the top and interfaces directly with the Vulkan loader. At the bottom of the stack sits the drivers.
// A driver can control one or more physical devices capable of rendering Vulkan, implement a conversion from Vulkan into
// a native graphics API (like MoltenVk (https://github.com/KhronosGroup/MoltenVK], or implement a fully software path that
// can be executed on a CPU to simulate a Vulkan device (like SwiftShader or LavaPipe). Remember, Vulkan-capable hardware may be
// graphics-based, compute-based, or both. Between the application and the drivers, the loader can inject any number of optional
// layers that provide special functionality. The loader is critical to managing the proper dispatching of Vulkan functions
// to the appropriate set of layers and drivers. The Vulkan object model allows the loader to insert layers into a call-chain
// so that the layers can process Vulkan functions prior to the driver being called.
//
// From
// https://github.com/KhronosGroup/Vulkan-Loader/blob/master/docs/LoaderApplicationInterface.md :
// There are several ways Vulkan functions may be interfaced through the loader:
//
// Vulkan Direct Exports (this + Dynamic Linking below is what this example program uses, the simpler way)
// The loader library on Windows, Linux, Android, and macOS will export all core Vulkan entry-points and all appropriate
// Window System Interface (WSI) entry-points. This is done to make it simpler to get started with Vulkan development. When
// an application links directly to the loader library in this way, the Vulkan calls are simple trampoline functions that jump
// to the appropriate dispatch table entry for the object they are given.
//
// Directly Linking to the Loader
// 1. Dynamic Linking
// The loader is distributed as a dynamic library (.dll on Windows or .so on Linux or .dylib on macOS) which gets installed
// to the system path for dynamic libraries. Furthermore, the dynamic library is generally installed to Windows systems as
// part of driver installation and is generally provided on Linux through the system package manager. This means that applications
// can usually expect a copy of the loader to be present on a system. If applications want to be completely sure that a loader
// is present, they can include a loader or runtime installer with their application.
//
// 2. Static Linking
// In previous versions of the loader, it was possible to statically link the loader. This was removed and is no longer possible. 
//
// 3. Indirectly Linking to the Loader (the more performant way, see runtime_dynlink.c)
// Applications are not required to link directly to the loader library, instead they can use the appropriate platform-specific
// dynamic symbol lookup on the loader library to initialize the application's own dispatch table. This allows an application
// to fail gracefully if the loader cannot be found. It also provides the fastest mechanism for the application to call Vulkan
// functions. An application only needs to query (via system calls such as dlsym) the address of vkGetInstanceProcAddr from the
// loader library. The application then uses vkGetInstanceProcAddr to load all functions available, such as vkCreateInstance,
// vkEnumerateInstanceExtensionProperties and vkEnumerateInstanceLayerProperties in a platform-independent way.

// -------------------------------------------------------------------------------------------
// The code below is based on "API without Secrets: Introduction to Vulkan" by Pawel Lapinski:
// https://www.intel.com/content/www/us/en/developer/articles/training/api-without-secrets-introduction-to-vulkan-part-1.html
// It represents an attempt to (a) rewrite the code in plain C, rather than C++ and (b) remove as much housekeeping code
// as possible, to the point of oversimplifying the structure of the program. The reason is this approach allows
// tighter focus on Vulkan, rather than drawing attention towards other aspects of the program.

// Note that I do not suggest using this program as an example of good coding practices. On the contrary,
// I strongly discourage following its coding "style". The sole reason of its existence is to provide a very
// basic example of Vulkan use.

// Before compilation, make sure you have VulkanSDK installed ( https://www.lunarg.com/vulkan-sdk/ )
//
// On Windows, edit Makefile - replace the path to Vulkan SDK in
// VULKAN_SDK_DIR = /C/VulkanSDK/1.2.189.2
// substituting the correct path to the installed VulkanSDK.
//
// Use 'make' to compile.
//
// Alternatively, on Windows you can use win_build.sh, replacing /C/VulkanSDK/1.2.189.2/
// with correct path to the installed VulkanSDK in both
// -I/C/VulkanSDK/1.2.189.2/Include
// and
// -L/C/VulkanSDK/1.2.189.2/Lib
//
// On Linux you can use linux_build.sh

#include <stdio.h>

// VK_ENABLE_BETA_EXTENSIONS enables VK_QUEUE_VIDEO_DECODE_BIT_KHR, VK_QUEUE_VIDEO_ENCODE_BIT_KHR
// enum VkQueueFlagBits variants (constants)
#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.h>

int main()
{
    // Now create a Vulkan instance
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;    // VkStructureType
    app_info.pNext = NULL;                                  // const void*
    app_info.pApplicationName = "Dynamic Loader";           // const char*
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // uint32_t
    app_info.pEngineName = "No Engine";                     // const char*
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);      // uint32_t
    app_info.apiVersion = VK_API_VERSION_1_0;               // uint32_t

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // VkStructureType
    create_info.pNext = NULL;                                   // const void*
    create_info.flags = 0;                                      // VkInstanceCreateFlags
    create_info.pApplicationInfo = &app_info;                   // const VkApplicationInfo*
    create_info.enabledLayerCount = 0;                          // uint32_t
    create_info.ppEnabledLayerNames = NULL;                     // const char* const*
    create_info.enabledExtensionCount = 0;                      // uint32_t
    create_info.ppEnabledExtensionNames = NULL;                 // const char * const*

    VkInstance vk_instance;
    if(vkCreateInstance(&create_info, NULL, &vk_instance) != VK_SUCCESS)
    {
        printf("Could not create Vulkan instance!\n");
        return -1000;
    }
    printf("Vulkan instance created\n");

    // -- Enumerate physical devices --
    uint32_t num_devices = 0;
    // Call vkEnumeratePhysicalDevices without the pointer to VkPhysicalDevice
    // array first (passing NULL) to get the number of devices in num_devices
    if(vkEnumeratePhysicalDevices(vk_instance, &num_devices, NULL) != VK_SUCCESS)
    {
        printf("Could get the number of physical devices\n");
        return -1000;
    }
    if(num_devices == 0)
    {
        printf("0 physical devices found\n");
        return -1000;
    }
    printf("Found %d physical device%s\n", num_devices, (num_devices > 1) ? "s" : "");

    // Now we have the number of physical devices, so we can allocate an array for them
    // https://stackoverflow.com/questions/1887097/why-arent-variable-length-arrays-part-of-the-c-standard
    // https://www.geeksforgeeks.org/variable-length-arrays-in-c-and-c/
    // https://gcc.gnu.org/onlinedocs/gcc/Variable-Length.html
    VkPhysicalDevice p_phys_devices[num_devices]; // <- "I believe GCC places it on the stack, like normal automatic variables.
    // It may or may not use dynamic allocation if the size is too large for the stack; I don't know myself."
    if(vkEnumeratePhysicalDevices(vk_instance, &num_devices, p_phys_devices) != VK_SUCCESS)
    {
        printf("Could not enumerate physical devices\n");
        return -1000;
    }
    // We can also skip the first call, if we provide preallocated array, and its length as the second parameter.
    // The number of devices we provided will be replaced by the actual number of enumerated physical devices (which will not
    // be greater than the value we provided).
    // Example: we don’t want to call this function twice. Our application supports up to
    // 10 devices and we provide this value along with a pointer to a static, 10-element array. The driver always returns the
    // number of actually enumerated devices. If there is none, zero is stored in the variable address we provided.
    // If there is any such device, we will also know that. We will not be able to tell if there are more than 10 devices.

    // Check each device's properties
    VkPhysicalDevice p_selected_phys_device = VK_NULL_HANDLE;
    uint32_t selected_queue_family_index = UINT32_MAX;

    VkPhysicalDeviceProperties dev_properties;
    const struct
    {
        VkVendorId vendor_id;
        const char vendor_name[64];
    } vendors[] =
    {
        // Vendors identified by their PCI vendor IDs, for the complete list see
        // https://pcisig.com/membership/member-companies
        { 0x1002, "AMD" },
        { 0x1010, "ImgTec" },
        { 0x10DE, "Nvidia" },
        { 0x13B5, "ARM" },
        { 0x5143, "Qualcomm" },
        { 0x8086, "Intel" },
        // Vendors identified by their Vulkan IDs, the list is at
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkVendorId.html
        { VK_VENDOR_ID_VIV, "VIV" },
        { VK_VENDOR_ID_VSI, "VSI" },
        { VK_VENDOR_ID_KAZAN, "KAZAN" },
        { VK_VENDOR_ID_CODEPLAY, "CODEPLAY" },
        { VK_VENDOR_ID_MESA, "MESA" },
        { VK_VENDOR_ID_POCL, "POCL" }
    };
    VkPhysicalDeviceFeatures dev_features;
    for(uint32_t i = 0; i < num_devices; i++)
    {
        printf("\n== Physical device %u ==\n", i);

        vkGetPhysicalDeviceProperties(p_phys_devices[i], &dev_properties);

        printf("Name: %s\n", dev_properties.deviceName);
        printf("Type: ");
        switch(dev_properties.deviceType)
        {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            printf("Other\n");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            printf("Integrated GPU\n");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            printf("Discrete GPU\n");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            printf("Virtual GPU\n");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            printf("CPU\n");
            break;
        default:
            printf("Unknown\n");
        }
        printf("Vulkan API: %u.%u.%u (variant %u)\n",
               VK_API_VERSION_MAJOR(dev_properties.apiVersion),
               VK_API_VERSION_MINOR(dev_properties.apiVersion),
               VK_API_VERSION_PATCH(dev_properties.apiVersion),
               VK_API_VERSION_VARIANT(dev_properties.apiVersion));
        printf("Driver version: %u\n", dev_properties.driverVersion);
        printf("Vendor ID: %x", dev_properties.vendorID);
        for(int i = 0; i < sizeof(vendors) / sizeof(vendors[0]); i++)
            if(vendors[i].vendor_id == dev_properties.vendorID)
            {
                printf(" (%s)", vendors[i].vendor_name);
                break;
            }
        printf("\n");
        printf("Device ID: %x\n", dev_properties.deviceID);

        printf("Some of the device limits:\n");
        printf("  maxImageDimension1D: %u\n", dev_properties.limits.maxImageDimension1D);
        printf("  maxImageDimension2D: %u\n", dev_properties.limits.maxImageDimension2D);
        printf("  maxImageDimension3D: %u\n", dev_properties.limits.maxImageDimension3D);
        printf("  maxImageDimensionCube: %u\n", dev_properties.limits.maxImageDimensionCube);
        printf("  maxTexelBufferElements: %u\n", dev_properties.limits.maxTexelBufferElements);
        printf("  sparseAddressSpaceSize: %lu\n", dev_properties.limits.sparseAddressSpaceSize);
        printf("  maxGeometryOutputVertices: %u\n", dev_properties.limits.maxGeometryOutputVertices );
        printf("  maxViewportDimensions: %u x %u\n",
               dev_properties.limits.maxViewportDimensions[0],
               dev_properties.limits.maxViewportDimensions[1]);
        printf("  maxFramebufferWidth: %u\n", dev_properties.limits.maxFramebufferWidth);
        printf("  maxFramebufferHeight: %u\n", dev_properties.limits.maxFramebufferHeight);
        printf("  pointSizeRange: [%f, %f]\n",
               dev_properties.limits. pointSizeRange[0],
               dev_properties.limits. pointSizeRange[1]);
        printf("  pointSizeGranularity: %f\n", dev_properties.limits.pointSizeGranularity);
        printf("  lineWidthRange: [%f, %f]\n",
               dev_properties.limits.lineWidthRange[0],
               dev_properties.limits.lineWidthRange[1]);
        printf("  lineWidthGranularity: %f\n", dev_properties.limits.lineWidthGranularity);

        // --- Physical device features ---
        // These must be explicitly enabled during logical device creation
        vkGetPhysicalDeviceFeatures(p_phys_devices[i], &dev_features);

        // Queue families
        uint32_t q_families_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(p_phys_devices[i], &q_families_count, NULL);
        printf("\nNumber of queue families: %u\n", q_families_count);

        VkQueueFamilyProperties q_fam_props[q_families_count];
        vkGetPhysicalDeviceQueueFamilyProperties(p_phys_devices[i], &q_families_count, q_fam_props);
        for(uint32_t j = 0; j < q_families_count; j++)
        {
            printf("  Queue family %u\n", j);
            printf("    Number of queues: %u\n", q_fam_props[j].queueCount);
            printf("    Flags:");
            if(q_fam_props[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                printf(" VK_QUEUE_GRAPHICS_BIT");
            if(q_fam_props[j].queueFlags & VK_QUEUE_COMPUTE_BIT)
                printf(" VK_QUEUE_COMPUTE_BIT");
            if(q_fam_props[j].queueFlags & VK_QUEUE_TRANSFER_BIT)
                printf(" VK_QUEUE_TRANSFER_BIT");
            if(q_fam_props[j].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
                printf(" VK_QUEUE_SPARSE_BINDING_BIT");
            if(q_fam_props[j].queueFlags & VK_QUEUE_PROTECTED_BIT)
                printf(" VK_QUEUE_PROTECTED_BIT");
#ifdef VK_ENABLE_BETA_EXTENSIONS
            if(q_fam_props[j].queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
                printf(" VK_QUEUE_VIDEO_DECODE_BIT_KHR");
            if(q_fam_props[j].queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
                printf(" VK_QUEUE_VIDEO_ENCODE_BIT_KHR");
#endif
            printf("\n");
        }
    }

    if(vk_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(vk_instance, NULL);
		printf("\nVulkan instance destroyed\n");
    }
    return 0;
}
