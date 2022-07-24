// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "../Instance.hpp"

#include <volk.h>
#include <vk_mem_alloc.h>

#include <vector>

namespace minte
{
	namespace backend
	{
		/**
		 * Vulkan instance class.
		 */
		class VulkanInstance final : public backend::Instance
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

			/**
			 * Get the instance.
			 *
			 * @return The instance.
			 */
			[[nodiscard]] VkInstance getInstance() const { return m_Instance; }

			/**
			 * Get the logical device.
			 *
			 * @return The logical device.
			 */
			[[nodsicard]] VkDevice getLogicalDevice() const { return m_LogicalDevice; }

			/**
			 * Get the physical device.
			 *
			 * @return The physical device.
			 */
			[[nodsicard]] VkPhysicalDevice getPhysicalDevice() const { return m_PhysicalDevice; }

			/**
			 * Get the device table.
			 *
			 * @return The device table.
			 */
			[[nodiscard]] const VolkDeviceTable& getDeviceTable() const { return m_DeviceTable; }

			/**
			 * Get the memory allocator.
			 *
			 * @return The allocator.
			 */
			[[nodiscard]] VmaAllocator getAllocator() const { return m_Allocator; }

			/**
			 * Get the graphics queue.
			 *
			 * @return The queue.
			 */
			[[nodiscard]] VulaknQueue getGraphicsQueue() const { return m_GraphicsQueue; }

			/**
			 * Get the transfer queue.
			 *
			 * @return The queue.
			 */
			[[nodiscard]] VulaknQueue getTransferQueue() const { return m_TransferQueue; }

			/**
			 * Get the compute queue.
			 *
			 * @return The queue.
			 */
			[[nodiscard]] VulaknQueue getComputeQueue() const { return m_ComputeQueue; }

			/**
			 * Change the image layout of an image.
			 *
			 * @param commandBuffer The command buffer to record all the commands.
			 * @param image The image to change the layout of.
			 * @param currentLayout The current layout of the image.
			 * @param newLayout The new layout to change to.
			 * @param aspectFlags The image aspect flags.
			 * @param mipLevels The image mip levels. Default is 1.
			 * @param layers The image layers. Default is 1.
			 */
			void changeImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout, VkImageAspectFlags aspectFlags, uint32_t mipLevels = 1, uint32_t layers = 1) const;

		private:
			/**
			 * Setup the instance.
			 */
			void setupInstance();

			/**
			 * Setup the device(s).
			 */
			void setupDevice();

			/**
			 * Setup the allocator.
			 */
			void setupAllocator();

		private:
			VkPhysicalDeviceProperties m_PhysicalDeviceProperties = {};

			VolkDeviceTable m_DeviceTable = {};

			std::vector<const char*> m_ValidationLayers;

			VkInstance m_Instance = VK_NULL_HANDLE;
			VkDebugUtilsMessengerEXT m_Debugger = VK_NULL_HANDLE;

			VkDevice m_LogicalDevice = VK_NULL_HANDLE;
			VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;

			VmaAllocator m_Allocator = nullptr;

			VulaknQueue m_GraphicsQueue = {};
			VulaknQueue m_TransferQueue = {};
			VulaknQueue m_ComputeQueue = {};
		};
	}
}