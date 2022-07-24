// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "../RenderTarget.hpp"
#include "VulkanInstance.hpp"

namespace minte
{
	namespace backend
	{
		/**
		 * Vulkan render target class.
		 */
		class VulkanRenderTarget final : public backend::RenderTarget
		{
			/**
			 * Vulkan attachment structure.
			 */
			struct VulkanAttachment final
			{
				VkImage m_Image = VK_NULL_HANDLE;
				VkImageView m_ImageView = VK_NULL_HANDLE;

				VmaAllocation m_ImageAllocation = nullptr;
				VkImageLayout m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			};

		public:
			/**
			 * Explicit constructor.
			 *
			 * @param pInstance The Vulkan instance pointer.
			 * @param width The width of the render target.
			 * @param height The height of the render target.
			 * @param antiAliasing The anti aliasing to use. Default is x1.
			 */
			explicit VulkanRenderTarget(const std::shared_ptr<VulkanInstance>& pInstance, uint32_t width, uint32_t height, AntiAliasing antiAliasing = AntiAliasing::X1);

			/**
			 * Destructor.
			 */
			~VulkanRenderTarget() override;

			/**
			 * Draw all the entities that are bound to the render target.
			 */
			void draw() override;

		private:
			/**
			 * Create a new attachment.
			 *
			 * @param format The image format.
			 * @param sampleCount The image multisample count.
			 * @param usageFlags The image usage flags.
			 * @param tiling The image tiling.
			 * @param aspectFlags The image view aspect flags.
			 * @return The created attachment.
			 */
			[[nodiscard]] VulkanAttachment createAttachment(VkFormat format, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usageFlags, VkImageTiling tiling, VkImageAspectFlags aspectFlags) const;

			/**
			 * Destroy an attachment.
			 *
			 * @param attachment The attachment to destroy.
			 */
			void destroyAttachment(const VulkanAttachment& attachment) const;

			/**
			 * Setup the render pass.
			 */
			void setupRenderPass();

			/**
			 * Setup the frame buffer.
			 */
			void setupFramebuffer();

			/**
			 * Setup the command pool and buffer.
			 */
			void setupCommandBuffer();

			/**
			 * Wait for the fence to finish execution.
			 */
			void waitForFence() const;

		private:
			VulkanAttachment m_ColorAttachment = {};	// The color attachment.
			VulkanAttachment m_EntityAttachment = {};	// This contains the entity IDs of all the drawn entities.
			VulkanAttachment m_DepthAttachment = {};	// The depth attachment.

			VkRenderPass m_RenderPass = VK_NULL_HANDLE;
			VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;

			VkCommandPool m_CommandPool = VK_NULL_HANDLE;
			VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
			VkFence m_Fence = VK_NULL_HANDLE;
		};
	}
}