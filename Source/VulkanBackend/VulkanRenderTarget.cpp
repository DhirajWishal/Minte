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
		m_EntityAttachment = createAttachment(VK_FORMAT_R32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
		m_DepthAttachment = createAttachment(VK_FORMAT_D16_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

		setupRenderPass();
		setupFramebuffer();
		setupCommandBuffer();
	}

	VulkanRenderTarget::~VulkanRenderTarget()
	{
		destroyAttachment(m_ColorAttachment);
		destroyAttachment(m_EntityAttachment);
		destroyAttachment(m_DepthAttachment);

		const auto pInstance = getInstance()->as<VulkanInstance>();
		pInstance->getDeviceTable().vkDestroyRenderPass(pInstance->getLogicalDevice(), m_RenderPass, VK_NULL_HANDLE);
		pInstance->getDeviceTable().vkDestroyFramebuffer(pInstance->getLogicalDevice(), m_Framebuffer, VK_NULL_HANDLE);
		pInstance->getDeviceTable().vkDestroyCommandPool(pInstance->getLogicalDevice(), m_CommandPool, VK_NULL_HANDLE);
		pInstance->getDeviceTable().vkDestroyFence(pInstance->getLogicalDevice(), m_Fence, VK_NULL_HANDLE);
	}

	void VulkanRenderTarget::draw()
	{
		const auto pInstance = getInstance()->as<VulkanInstance>();

		// Begin command buffer.
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pNext = VK_NULL_HANDLE;
		beginInfo.pInheritanceInfo = VK_NULL_HANDLE;

		MINTE_VK_ASSERT(pInstance->getDeviceTable().vkBeginCommandBuffer(m_CommandBuffer, &beginInfo), "Failed to begin command buffer!");

		// Setup the clear colors.
		std::array<VkClearValue, 3> clearColors;
		clearColors[0].color.float32[0] = 0.0f;
		clearColors[0].color.float32[1] = 0.0f;
		clearColors[0].color.float32[2] = 0.0f;
		clearColors[0].color.float32[3] = 0.0f;

		clearColors[1].color.float32[0] = 0.0f;
		clearColors[1].color.float32[1] = 0.0f;
		clearColors[1].color.float32[2] = 0.0f;
		clearColors[1].color.float32[3] = 0.0f;

		clearColors[2].depthStencil.depth = 1.0f;
		clearColors[2].depthStencil.stencil = 0.0f;

		// Bind the render target.
		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = VK_NULL_HANDLE;
		renderPassBeginInfo.renderPass = m_RenderPass;
		renderPassBeginInfo.framebuffer = m_Framebuffer;
		renderPassBeginInfo.renderArea.extent = VkExtent2D{ getWidth(), getHeight() };
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
		renderPassBeginInfo.pClearValues = clearColors.data();

		pInstance->getDeviceTable().vkCmdBeginRenderPass(m_CommandBuffer, &renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

		// Bind the pipeline.
		// Draw the entities.

		// Unbind the render target.
		pInstance->getDeviceTable().vkCmdEndRenderPass(m_CommandBuffer);

		// Copy the color, depth and picking images to the buffers.
		// First, change the image layout.
		pInstance->changeImageLayout(m_CommandBuffer, m_ColorAttachment.m_Image, m_ColorAttachment.m_CurrentLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
		pInstance->changeImageLayout(m_CommandBuffer, m_EntityAttachment.m_Image, m_EntityAttachment.m_CurrentLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
		pInstance->changeImageLayout(m_CommandBuffer, m_DepthAttachment.m_Image, m_DepthAttachment.m_CurrentLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

		// Perform the copy.
		VkBufferImageCopy imageCopy = {};
		imageCopy.imageExtent = { getWidth(), getHeight(), 1 };
		imageCopy.imageOffset = { 0, 0, 0 };
		imageCopy.imageSubresource.baseArrayLayer = 0;
		imageCopy.imageSubresource.layerCount = 1;
		imageCopy.imageSubresource.mipLevel = 0;
		imageCopy.bufferOffset = 0;
		imageCopy.bufferImageHeight = getHeight();
		imageCopy.bufferRowLength = getWidth();

		imageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		pInstance->getDeviceTable().vkCmdCopyImageToBuffer(m_CommandBuffer, m_ColorAttachment.m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_ColorAttachment.m_Buffer, 1, &imageCopy);
		pInstance->getDeviceTable().vkCmdCopyImageToBuffer(m_CommandBuffer, m_EntityAttachment.m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_EntityAttachment.m_Buffer, 1, &imageCopy);

		imageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		pInstance->getDeviceTable().vkCmdCopyImageToBuffer(m_CommandBuffer, m_DepthAttachment.m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_DepthAttachment.m_Buffer, 1, &imageCopy);

		// Change them back to how they were.
		pInstance->changeImageLayout(m_CommandBuffer, m_ColorAttachment.m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT);
		pInstance->changeImageLayout(m_CommandBuffer, m_EntityAttachment.m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT);
		pInstance->changeImageLayout(m_CommandBuffer, m_DepthAttachment.m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);

		// Set the correct layouts.
		m_ColorAttachment.m_CurrentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		m_EntityAttachment.m_CurrentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		m_DepthAttachment.m_CurrentLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		// End the command buffer.
		MINTE_VK_ASSERT(pInstance->getDeviceTable().vkEndCommandBuffer(m_CommandBuffer), "Failed to end command buffer!");

		// Submit.
		const VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = VK_NULL_HANDLE;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;
		submitInfo.pWaitDstStageMask = &waitStageMask;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = VK_NULL_HANDLE;

		MINTE_VK_ASSERT(pInstance->getDeviceTable().vkQueueSubmit(pInstance->getGraphicsQueue().m_Queue, 1, &submitInfo, m_Fence), "Failed to submit the queue!");

		// Wait for the fence to finish execution.
		waitForFence();
	}

	void VulkanRenderTarget::setupRenderPass()
	{
		// Resolve attachments.
		std::array<VkAttachmentReference, 3> attachmentReferences;
		std::array<VkAttachmentDescription, 3> attachmentDescriptions;

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

		// Entity attachment.
		attachmentDescriptions[1].flags = 0;
		attachmentDescriptions[1].format = VK_FORMAT_R32_SFLOAT;
		attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;	// TODO
		attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		attachmentReferences[1].attachment = 1;
		attachmentReferences[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// Depth attachment.
		attachmentDescriptions[2].flags = 0;
		attachmentDescriptions[2].format = VK_FORMAT_D16_UNORM;
		attachmentDescriptions[2].samples = VK_SAMPLE_COUNT_1_BIT;	// TODO
		attachmentDescriptions[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescriptions[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescriptions[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescriptions[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescriptions[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescriptions[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		attachmentReferences[2].attachment = 2;
		attachmentReferences[2].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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
		subpassDescription.colorAttachmentCount = static_cast<uint32_t>(attachmentReferences.size() - 1);
		subpassDescription.pColorAttachments = attachmentReferences.data();
		subpassDescription.pResolveAttachments = VK_NULL_HANDLE;
		subpassDescription.pDepthStencilAttachment = &attachmentReferences[2];
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = VK_NULL_HANDLE;

		// Create the render target.
		VkRenderPassCreateInfo renderPassCreateInfo = {};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.pNext = VK_NULL_HANDLE;
		renderPassCreateInfo.flags = 0;
		renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
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
		VmaAllocationCreateInfo imageAllocationCreateInfo = {};
		imageAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

		// Create the image.
		MINTE_VK_ASSERT(vmaCreateImage(pInstance->getAllocator(), &imageCreateInfo, &imageAllocationCreateInfo, &attachment.m_Image, &attachment.m_ImageAllocation, VK_NULL_HANDLE), "Failed to create the image!");

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

		// Get the image memory requirements.
		VkMemoryRequirements imageMemoryRequirements = {};
		vkGetImageMemoryRequirements(pInstance->getLogicalDevice(), attachment.m_Image, &imageMemoryRequirements);

		// Create the copy buffer.
		VkBufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.pNext = VK_NULL_HANDLE;
		createInfo.flags = 0;
		createInfo.size = imageMemoryRequirements.size;
		createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = VK_NULL_HANDLE;

		VmaAllocationCreateInfo bufferAllocationCreateInfo = {};
		bufferAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		bufferAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;

		MINTE_VK_ASSERT(vmaCreateBuffer(pInstance->getAllocator(), &createInfo, &bufferAllocationCreateInfo, &attachment.m_Buffer, &attachment.m_BufferAllocation, VK_NULL_HANDLE), "Failed to create the buffer!");

		return attachment;
	}

	void VulkanRenderTarget::destroyAttachment(const VulkanAttachment& attachment) const
	{
		const auto pInstance = getInstance()->as<VulkanInstance>();

		vmaDestroyImage(pInstance->getAllocator(), attachment.m_Image, attachment.m_ImageAllocation);
		vmaDestroyBuffer(pInstance->getAllocator(), attachment.m_Buffer, attachment.m_BufferAllocation);
		pInstance->getDeviceTable().vkDestroyImageView(pInstance->getLogicalDevice(), attachment.m_ImageView, VK_NULL_HANDLE);
	}

	void VulkanRenderTarget::setupFramebuffer()
	{
		std::array<VkImageView, 3> imageViews = { m_ColorAttachment.m_ImageView, m_EntityAttachment.m_ImageView, m_DepthAttachment.m_ImageView };

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.pNext = VK_NULL_HANDLE;
		frameBufferCreateInfo.flags = 0;
		frameBufferCreateInfo.renderPass = m_RenderPass;
		frameBufferCreateInfo.width = getWidth();
		frameBufferCreateInfo.height = getHeight();
		frameBufferCreateInfo.layers = 1;
		frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
		frameBufferCreateInfo.pAttachments = imageViews.data();

		const auto pInstance = getInstance()->as<VulkanInstance>();
		MINTE_VK_ASSERT(pInstance->getDeviceTable().vkCreateFramebuffer(pInstance->getLogicalDevice(), &frameBufferCreateInfo, VK_NULL_HANDLE, &m_Framebuffer), "Failed to create the frame buffer!");
	}

	void VulkanRenderTarget::setupCommandBuffer()
	{
		const auto pInstance = getInstance()->as<VulkanInstance>();

		// Create the command pool.
		VkCommandPoolCreateInfo commandPoolCreateInfo = {};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.pNext = VK_NULL_HANDLE;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.queueFamilyIndex = pInstance->getGraphicsQueue().m_Family;

		MINTE_VK_ASSERT(pInstance->getDeviceTable().vkCreateCommandPool(pInstance->getLogicalDevice(), &commandPoolCreateInfo, VK_NULL_HANDLE, &m_CommandPool), "Failed to create the command pool!");

		// Allocate the command buffers.
		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.pNext = VK_NULL_HANDLE;
		allocateInfo.commandPool = m_CommandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = 1;

		MINTE_VK_ASSERT(pInstance->getDeviceTable().vkAllocateCommandBuffers(pInstance->getLogicalDevice(), &allocateInfo, &m_CommandBuffer), "Failed to allocate command buffers!");

		// Create the fence.
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = 0;
		fenceCreateInfo.pNext = VK_NULL_HANDLE;

		MINTE_VK_ASSERT(pInstance->getDeviceTable().vkCreateFence(pInstance->getLogicalDevice(), &fenceCreateInfo, nullptr, &m_Fence), "Failed to create fence!");
	}

	void VulkanRenderTarget::waitForFence() const
	{
		const auto pInstance = getInstance()->as<VulkanInstance>();
		MINTE_VK_ASSERT(pInstance->getDeviceTable().vkWaitForFences(pInstance->getLogicalDevice(), 1, &m_Fence, VK_TRUE, std::numeric_limits<uint64_t>::max()), "Failed to wait for the fence!");
		MINTE_VK_ASSERT(pInstance->getDeviceTable().vkResetFences(pInstance->getLogicalDevice(), 1, &m_Fence), "Failed to reset fence!");
	}
}