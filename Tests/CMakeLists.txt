# Copyright (c) 2022 Dhiraj Wishal

# Set the basic project information.
project(
	MinteTests
	VERSION 1.0.0
	DESCRIPTION "Sample application."
)

# Add the library.
add_executable(
	MinteTests

	"Main.cpp"

	"Layers/HeadsUpDisplay.hpp"
	"Layers/HeadsUpDisplay.cpp"
)

# Add the optick static library as a target link.
target_link_libraries(MinteTests Minte)

# Make sure to specify the C++ standard to C++20.
set_property(TARGET MinteTests PROPERTY CXX_STANDARD 20)

# If we are on MSVC, we can use the Multi Processor Compilation option.
if (MSVC)
	target_compile_options(MinteTests PRIVATE "/MP")	
endif ()
