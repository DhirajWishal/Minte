// Copyright (c) 2022 Dhiraj Wishal

#include "Minte/Minte.hpp"

#include "VulkanBackend/VulkanInstance.hpp"

namespace minte
{
	Minte::Minte()
		: m_pInstance(std::make_shared<backend::VulkanInstance>())
	{
	}
}