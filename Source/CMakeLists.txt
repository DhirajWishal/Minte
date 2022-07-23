# Copyright (c) 2022 Dhiraj Wishal

# Set the basic project information.
project(
	Minte
	VERSION 1.0.0
	DESCRIPTION "Minte library"
)

# Add the library.
add_library(
	Minte
	STATIC

	"${CMAKE_SOURCE_DIR}/Include/Minte/BackendError.hpp"
	"${CMAKE_SOURCE_DIR}/Include/Minte/DataTypes.hpp"
	"${CMAKE_SOURCE_DIR}/Include/Minte/Instance.hpp"

	"Instance.cpp"

	# Vulkan backend
	"VulkanBackend/VulkanInstance.hpp"
	"VulkanBackend/VulkanInstance.cpp"
	"VulkanBackend/VulkanMacros.hpp"
)

# Set the include directories.
target_include_directories(
	Minte 

	PUBLIC ${VULKAN_HEADERS_INCLUDE_DIR} 
	PUBLIC ${VOLK_INCLUDE_DIR} 
	PUBLIC ${VMA_INCLUDE_DIR} 
)

# Add the target links.
target_link_libraries(Minte volk)

# Make sure to specify the C++ standard to C++20.
set_property(TARGET Minte PROPERTY CXX_STANDARD 20)

# If we are on MSVC, we can use the Multi Processor Compilation option.
if (MSVC)
	target_compile_options(Minte PRIVATE "/MP")	
endif ()