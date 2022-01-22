SHELL = /bin/sh
CC = gcc
CFLAGS = -O2
LDFLAGS = -Wl,--strip-all
LDIRFLAGS =
SRC = runtime_dynlink.c

ifeq ($(OS),Windows_NT)
	# !!! On Windows replace this path with correct path to Vulkan SDK !!!
	VULKAN_SDK_DIR = /C/VulkanSDK/1.2.189.2
	OUTNAME = runtime_dynlink.exe
	IFLAGS = -I$(VULKAN_SDK_DIR)/Include
	DFLAGS = -DVK_USE_PLATFORM_WIN32_KHR
	RM = rm -f
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		OUTNAME = runtime_dynlink
		IFLAGS =
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
	$(CC) $(CFLAGS) $(IFLAGS) -o $(OUTNAME) $(LDFLAGS) $(LDIRFLAGS) $(DFLAGS) $(SRC)

run: $(OUTNAME)
	./$(OUTNAME)

clean:
	$(RM) ./$(OUTNAME)
