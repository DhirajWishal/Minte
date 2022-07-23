// Copyright (c) 2022 Dhiraj Wishal

#include "Minte/Instance.hpp"

#include "Layers/HeadsUpDisplay.hpp"

auto main(int argc, char** argv) -> int
try
{
	auto pInstance = minte::CreateInstance();
	auto hud = HeadsUpDisplay(pInstance);
}
catch (...)
{

}