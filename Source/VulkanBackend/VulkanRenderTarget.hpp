// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "Minte/Backend/RenderTarget.hpp"
#include "VulkanInstance.hpp"

namespace minte
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

			VmaAllocation m_Allocation = nullptr;
			VkImageLayout m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		};

	public:
		/**
		 * Explicit constructor.
		 *
		 * @param pInstance The Vulkan instance pointer.
		 * @param width The width of the render target.
		 * @param height The height of the render target.
		 */
		explicit VulkanRenderTarget(const std::shared_ptr<VulkanInstance>& pInstance, uint32_t width, uint32_t height);

		/**
		 * Destructor.
		 */
		~VulkanRenderTarget() override;

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
		 * Create the render pass.
		 */
		void createRenderPass();

		/**
		 * Create the frame buffer.
		 */
		void createFramebuffer();

	private:
		VulkanAttachment m_ColorAttachment = {};
		VulkanAttachment m_DepthAttachment = {};

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
	};
}