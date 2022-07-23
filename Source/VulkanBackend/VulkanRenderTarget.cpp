// Copyright (c) 2022 Dhiraj Wishal

#include "VulkanRenderTarget.hpp"
#include "VulkanMacros.hpp"

#include <array>

namespace minte
{
	VulkanRenderTarget::VulkanRenderTarget(const std::shared_ptr<VulkanInstance>& pInstance, uint32_t width, uint32_t height)
		: backend::RenderTarget(pInstance, width, height)
	{
		m_ColorAttachment = createAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
		m_DepthAttachment = createAttachment(VK_FORMAT_D16_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

		createRenderPass();
		createFramebuffer();
	}

	VulkanRenderTarget::~VulkanRenderTarget()
	{
		destroyAttachment(m_ColorAttachment);
		destroyAttachment(m_DepthAttachment);

		const auto pInstance = getInstance()->as<VulkanInstance>();
		pInstance->getDeviceTable().vkDestroyRenderPass(pInstance->getLogicalDevice(), m_RenderPass, VK_NULL_HANDLE);
		pInstance->getDeviceTable().vkDestroyFramebuffer(pInstance->getLogicalDevice(), m_Framebuffer, VK_NULL_HANDLE);
	}

	void VulkanRenderTarget::createRenderPass()
	{
		// Resolve attachments.
		std::array<VkAttachmentReference, 2> attachmentReferences;
		std::array<VkAttachmentDescription, 2> attachmentDescriptions;

		// Color attachment.
		attachmentDescriptions[0].flags = 0;
		attachmentDescriptions[0].format = VK_FORMAT_R8G8B8A8_UNORM;
		attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;	// TODO
		attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		attachmentReferences[0].attachment = 0;
		attachmentReferences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// Depth attachment.
		attachmentDescriptions[1].flags = 0;
		attachmentDescriptions[1].format = VK_FORMAT_D16_UNORM;
		attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;	// TODO
		attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		attachmentReferences[1].attachment = 1;
		attachmentReferences[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Create the subpass dependencies.
		std::array<VkSubpassDependency, 2> subpassDependencies;
		subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependencies[0].dstSubpass = 0;
		subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		subpassDependencies[1].srcSubpass = 0;
		subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create the subpass description.
		VkSubpassDescription subpassDescription = {};
		subpassDescription.flags = 0;
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = VK_NULL_HANDLE;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &attachmentReferences[0];
		subpassDescription.pResolveAttachments = VK_NULL_HANDLE;
		subpassDescription.pDepthStencilAttachment = &attachmentReferences[1];
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = VK_NULL_HANDLE;

		// Create the render target.
		VkRenderPassCreateInfo renderPassCreateInfo = {};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.pNext = VK_NULL_HANDLE;
		renderPassCreateInfo.flags = 0;
		renderPassCreateInfo.attachmentCount = 2;
		renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		renderPassCreateInfo.dependencyCount = 2;
		renderPassCreateInfo.pDependencies = subpassDependencies.data();

		const auto pInstance = getInstance()->as<VulkanInstance>();
		MINTE_VK_ASSERT(pInstance->getDeviceTable().vkCreateRenderPass(pInstance->getLogicalDevice(), &renderPassCreateInfo, VK_NULL_HANDLE, &m_RenderPass), "Failed to create render pass!");
	}

	minte::VulkanRenderTarget::VulkanAttachment VulkanRenderTarget::createAttachment(VkFormat format, VkSampleCountFlagBits sampleCount, VkImageUsageFlags usageFlags, VkImageTiling tiling, VkImageAspectFlags aspectFlags) const
	{
		VulkanAttachment attachment;
		const auto pInstance = getInstance()->as<VulkanInstance>();

		// Setup image create info structure.
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = VK_NULL_HANDLE;
		imageCreateInfo.flags = 0;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.extent.width = getWidth();
		imageCreateInfo.extent.height = getHeight();
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = sampleCount;
		imageCreateInfo.tiling = tiling;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.queueFamilyIndexCount = 0;
		imageCreateInfo.pQueueFamilyIndices = VK_NULL_HANDLE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.usage =
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT |	// We might use it to transfer data from this image.
			VK_IMAGE_USAGE_TRANSFER_DST_BIT |	// We might use it to transfer data to this image.
			usageFlags;

		// Setup the allocation info.
		VmaAllocationCreateInfo allocationCreateInfo = {};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

		// Create the image.
		MINTE_VK_ASSERT(vmaCreateImage(pInstance->getAllocator(), &imageCreateInfo, &allocationCreateInfo, &attachment.m_Image, &attachment.m_Allocation, VK_NULL_HANDLE), "Failed to create the image!");

		// Create the image view.
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext = VK_NULL_HANDLE;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.image = attachment.m_Image;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = format;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		MINTE_VK_ASSERT(pInstance->getDeviceTable().vkCreateImageView(pInstance->getLogicalDevice(), &imageViewCreateInfo, VK_NULL_HANDLE, &attachment.m_ImageView), "Failed to create the image view!");

		return attachment;
	}

	void VulkanRenderTarget::destroyAttachment(const VulkanAttachment& attachment) const
	{
		const auto pInstance = getInstance()->as<VulkanInstance>();

		vmaDestroyImage(pInstance->getAllocator(), attachment.m_Image, attachment.m_Allocation);
		pInstance->getDeviceTable().vkDestroyImageView(pInstance->getLogicalDevice(), attachment.m_ImageView, VK_NULL_HANDLE);
	}

	void VulkanRenderTarget::createFramebuffer()
	{
		std::array<VkImageView, 2> imageViews = { m_ColorAttachment.m_ImageView, m_DepthAttachment.m_ImageView };

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.pNext = VK_NULL_HANDLE;
		frameBufferCreateInfo.flags = 0;
		frameBufferCreateInfo.renderPass = m_RenderPass;
		frameBufferCreateInfo.width = getWidth();
		frameBufferCreateInfo.height = getHeight();
		frameBufferCreateInfo.layers = 1;
		frameBufferCreateInfo.attachmentCount = 2;
		frameBufferCreateInfo.pAttachments = imageViews.data();

		const auto pInstance = getInstance()->as<VulkanInstance>();
		MINTE_VK_ASSERT(pInstance->getDeviceTable().vkCreateFramebuffer(pInstance->getLogicalDevice(), &frameBufferCreateInfo, VK_NULL_HANDLE, &m_Framebuffer), "Failed to create the frame buffer!");
	}
}