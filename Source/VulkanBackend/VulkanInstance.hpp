// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "Minte/Instance.hpp"

#include <volk.h>

#include <vector>

namespace minte
{
	/**
	 * Vulkan instance class.
	 */
	class VulkanInstance final : public Instance
	{
		/**
		 * Vulkan Queue structure.
		 */
		struct VulaknQueue final
		{
			VkQueue m_Queue = VK_NULL_HANDLE;
			uint32_t m_Family = 0;
		};

	public:
		/**
		 * Default constructor.
		 */
		VulkanInstance();

		/**
		 * Destructor.
		 */
		~VulkanInstance() override;

	private:
		/**
		 * Setup the instance.
		 */
		void setupInstance();

		/**
		 * Setup the device(s).
		 */
		void setupDevice();

	private:
		VkPhysicalDeviceProperties m_PhysicalDeviceProperties = {};

		VolkDeviceTable m_DeviceTable = {};

		std::vector<const char*> m_ValidationLayers;

		VkInstance m_Instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_Debugger = VK_NULL_HANDLE;

		VkDevice m_LogicalDevice = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;

		VulaknQueue m_GraphicsQueue = {};
		VulaknQueue m_TransferQueue = {};
		VulaknQueue m_ComputeQueue = {};
	};
}