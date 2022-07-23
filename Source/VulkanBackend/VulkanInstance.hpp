// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "Minte/Instance.hpp"

namespace minte
{
	/**
	 * Vulkan instance class.
	 */
	class VulkanInstance final : public Instance
	{
	public:
		/**
		 * Default constructor.
		 */
		VulkanInstance();

		/**
		 * Destructor.
		 */
		~VulkanInstance() override;
	};
}