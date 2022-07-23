// Copyright (c) 2022 Dhiraj Wishal

#include "Minte/Minte.hpp"

#include "Layers/HeadsUpDisplay.hpp"

#include <iostream>

auto main(int argc, char** argv) -> int
try
{
	minte::Minte instance;
	auto hud = HeadsUpDisplay(instance);

	while (true) hud.update();
}
catch (std::runtime_error& error)
{
	std::cout << "Error occurred: " << error.what() << std::endl;
}