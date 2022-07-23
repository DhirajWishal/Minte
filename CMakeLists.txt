# Copyright (c) 2022 Dhiraj Wishal
# Main build script for Minte.

# Set the minimum required CMake version.
cmake_minimum_required(VERSION 3.22.2)

# Set the basic project description.
project(
	Minte
	VERSION 1.0.0 
	DESCRIPTION "Object-Oriented GUI library based on modern C++."
)

# Lets tell CMake to add the default ALL_BUILD, ZERO_CHECK and INSTALL to a group.
# This way we can make things much more simpler for Visual Studio.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(PREDEFINED_TARGETS_FOLDER "PredefinedTargets")

# Set the basic third party directory variables.
# Set the Vulkan header include directory.
set(VULKAN_HEADERS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/Vulkan-Headers/include)

# Set the Vulkan Memory Manager header include directory.
set(VMA_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/VulkanMemoryAllocator/include)

# Set the volk meta loader include directory.
set(VOLK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/volk)

# Set the SDL include, library and binary data.
set(SDL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/SDL/include)

# Add the SDL library as a subdirectory.
add_subdirectory(ThirdParty/SDL)

# Same as before, lets make a target library for this.
add_library(
	volk
	STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/volk/volk.c
)

# Add the STB submodule include directory.
set(STB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/stb)

# We need to set these because, it's not MSVC.
if(NOT MSVC)
	set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
	set(CMAKE_THREAD_LIBS_INIT "-lpthread")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	set(CMAKE_HAVE_THREADS_LIBRARY 1)
	set(CMAKE_USE_WIN32_THREADS_INIT 0)
	set(CMAKE_USE_PTHREADS_INIT 1)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
endif ()

# Add the Vulkan headers as a target include directory.
target_include_directories(volk PUBLIC ${VULKAN_HEADERS_INCLUDE_DIR})

# Set global compile definitions.
add_compile_definitions(
	$<$<CONFIG:Debug>:MINTE_DEBUG>
	$<$<CONFIG:Release>:MINTE_RELEASE>

	$<$<CONFIG:RelWithDebInfo>:MINTE_DEBUG>
	$<$<CONFIG:MinSizeRel>:MINTE_RELEASE>
	
	$<$<PLATFORM_ID:Windows>:MINTE_PLATFORM_WINDOWS>
	$<$<PLATFORM_ID:Linux>:MINTE_PLATFORM_LINUX>
	$<$<PLATFORM_ID:Darwin>:MINTE_PLATFORM_MAC>
)

# Set the central include directory.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Include)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Source/Minte)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Source/VulkanBackend)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Tests)

# Set the startup project for Visual Studio and set multi processor compilation for other projects that we build.
if (MSVC) 
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Minte)
	target_compile_options(volk PRIVATE "/MP")	
endif ()