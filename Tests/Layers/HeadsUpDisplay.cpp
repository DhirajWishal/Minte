// Copyright (c) 2022 Dhiraj Wishal

#include "HeadsUpDisplay.hpp"

#include "Minte/Backend/VulkanBackend/VulkanRenderTarget.hpp"

HeadsUpDisplay::HeadsUpDisplay(minte::Minte parent)
	: minte::Layer(parent, std::make_unique<minte::backend::VulkanRenderTarget>(parent.getInstanceAs<minte::backend::VulkanInstance>(), 1280, 720))
{
}
