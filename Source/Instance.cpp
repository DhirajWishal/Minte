// Copyright (c) 2022 Dhiraj Wishal

#include "Minte/Instance.hpp"

#include "VulkanBackend/VulkanInstance.hpp"

namespace minte
{
	std::shared_ptr<Instance> CreateInstance()
	{
		return std::make_shared<VulkanInstance>();
	}
}