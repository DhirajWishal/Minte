// Copyright (c) 2022 Dhiraj Wishal

#include "Minte/Minte.hpp"

#include "Minte/Backend/VulkanBackend/VulkanInstance.hpp"

#include "Layers/HeadsUpDisplay.hpp"

#include <iostream>

auto main(int argc, char** argv) -> int
try
{
	auto instance = minte::Minte(std::make_shared<minte::backend::VulkanInstance>());
	auto hud = HeadsUpDisplay(instance);

	while (true) const auto images = hud.update();
}
catch (std::runtime_error& error)
{
	std::cout << "Error occurred: " << error.what() << std::endl;
}