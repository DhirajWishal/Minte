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

	"${CMAKE_SOURCE_DIR}/Include/Minte/DataTypes.hpp"
	"${CMAKE_SOURCE_DIR}/Include/Minte/Minte.hpp"
	"${CMAKE_SOURCE_DIR}/Include/Minte/Layer.hpp"
	"${CMAKE_SOURCE_DIR}/Include/Minte/Drawable.hpp"

	"Layer.cpp"
	"Minte.cpp"
)

# Make sure to specify the C++ standard to C++20.
set_property(TARGET Minte PROPERTY CXX_STANDARD 20)

# Add the backends.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/VulkanBackend)

# If we are on MSVC, we can use the Multi Processor Compilation option.
if (MSVC)
	target_compile_options(Minte PRIVATE "/MP")	
endif ()