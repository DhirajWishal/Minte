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

	"${CMAKE_SOURCE_DIR}/Include/VulkanBackend/VulkanInstance.hpp"
	"${CMAKE_SOURCE_DIR}/Include/VulkanBackend/VulkanMacros.hpp"
	"${CMAKE_SOURCE_DIR}/Include/VulkanBackend/VulkanRenderTarget.hpp"
	
	"VulkanInstance.cpp"
	"VulkanRenderTarget.cpp"

	"vk_mem_alloc.cpp"
)

# Set the include directories.
target_include_directories(
	MinteVulkanBackend 

	PUBLIC ${VULKAN_HEADERS_INCLUDE_DIR} 
	PUBLIC ${VOLK_INCLUDE_DIR} 
	PUBLIC ${VMA_INCLUDE_DIR} 
)

# Add the target links.
target_link_libraries(MinteVulkanBackend volk)

# Make sure to specify the C++ standard to C++20.
set_property(TARGET MinteVulkanBackend PROPERTY CXX_STANDARD 20)

# If we are on MSVC, we can use the Multi Processor Compilation option.
if (MSVC)
	target_compile_options(MinteVulkanBackend PRIVATE "/MP")	
endif ()