# Copyright (c) 2022 Dhiraj Wishal

# Set the basic project information.
project(
	MinteVulkanBackend
	VERSION 1.0.0
	DESCRIPTION "Minte library"
)

# Add the library.
add_library(
	MinteVulkanBackend
	STATIC

	"${CMAKE_SOURCE_DIR}/Include/Minte/Backend/VulkanBackend/VulkanInstance.hpp"
	"${CMAKE_SOURCE_DIR}/Include/Minte/Backend/VulkanBackend/VulkanMacros.hpp"
	"${CMAKE_SOURCE_DIR}/Include/Minte/Backend/VulkanBackend/VulkanRenderTarget.hpp"
	"${CMAKE_SOURCE_DIR}/Include/Minte/Backend/VulkanBackend/VulkanWindow.hpp"
	"${CMAKE_SOURCE_DIR}/Include/Minte/Backend/VulkanBackend/VulkanImageBuffer.hpp"
	
	"VulkanInstance.cpp"
	"VulkanRenderTarget.cpp"
	"VulkanWindow.cpp"
	"VulkanImageBuffer.cpp"

	"vk_mem_alloc.cpp"
)

# Set the include directories.
target_include_directories(
	MinteVulkanBackend 

	PUBLIC ${VULKAN_HEADERS_INCLUDE_DIR} 
	PUBLIC ${VOLK_INCLUDE_DIR} 
	PUBLIC ${VMA_INCLUDE_DIR} 
	PUBLIC ${SDL_INCLUDE_DIR}
	PRIVATE ${SPDLOG_INCLUDE_DIR}
)

# Add the target links.
target_link_libraries(MinteVulkanBackend volk SDL2)

# Make sure to specify the C++ standard to C++20.
set_property(TARGET MinteVulkanBackend PROPERTY CXX_STANDARD 20)

# If we are on MSVC, we can use the Multi Processor Compilation option.
if (MSVC)
	target_compile_options(MinteVulkanBackend PRIVATE "/MP")	
endif ()