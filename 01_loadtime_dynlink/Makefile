SHELL = /bin/sh
CC = gcc
CFLAGS = -O2
LDFLAGS = -Wl,--strip-all
SRC = loadtime_dynlink.c

ifeq ($(OS),Windows_NT)
	# The executable file will depend on msvcrt.dll (check with `ldd ./loadtime_dynlink.exe`)
	# https://stackoverflow.com/questions/3943412/can-i-link-msvcrt-statically-with-mingw
	# https://stackoverflow.com/questions/28783079/mingw-mingw64-linking-and-dependency-on-msvcrt-dll
	# https://yongweiwu.wordpress.com/2016/05/27/msvcrt-dll-console-io-bug/
	# https://sourceforge.net/p/mingw-w64/wiki2/The%20case%20against%20msvcrt.dll/
	# I believe that MinGW doesn't use the static runtime library for copyright reasons.
	# You can maybe try to use newlib (http://sourceware.org/newlib/) to create an executable that doesn't link to msvcrt.dll
	# +1 for copyright avoidance. That is a clever side effect of it already being on every Windows system.
	# Programs built with MinGW require it, but never have to distribute it. Hence, no copyright issues.

	# !!! On Windows replace this path with correct path to Vulkan SDK !!!
	VULKAN_SDK_DIR = /C/VulkanSDK/1.2.189.2
	OUTNAME = loadtime_dynlink.exe
	IFLAGS = -I$(VULKAN_SDK_DIR)/Include
	LIBS = -lvulkan-1
	LDIRFLAGS = -L$(VULKAN_SDK_DIR)/Lib
	DFLAGS = -DVK_USE_PLATFORM_WIN32_KHR
	RM = rm -f
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		OUTNAME = loadtime_dynlink
		IFLAGS =
		LIBS = -lvulkan
		LDIRFLAGS =
		DFLAGS = -DVK_USE_PLATFORM_XCB_KHR
		RM = rm -f
#	else ifeq ($(UNAME_S),Darwin))
#		I don't have access to MacOS, you're on your own :)
	endif
endif

#LDFLAGS = -lvulkan -lglfw -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
# https://stackoverflow.com/questions/13768515/how-to-do-static-linking-of-libwinpthread-1-dll-in-mingw
#LDFLAGS = -lvulkan-1 -lglfw3 -lgdi32 -luser32 -lkernel32 -static-libgcc -static-libstdc++ -Wl,-Bstatic -lpthread

.PHONY: run clean

$(OUTNAME): $(SRC)
	$(CC) $(CFLAGS) $(IFLAGS) $(SRC) $(DFLAGS) -o $(OUTNAME) $(LDFLAGS) $(LDIRFLAGS) $(LIBS)

run: $(OUTNAME)
	./$(OUTNAME)

clean:
	$(RM) ./$(OUTNAME)
