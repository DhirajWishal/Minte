// Copyright (c) 2022 Dhiraj Wishal

#include "Minte/Backend/VulkanBackend/VulkanWindow.hpp"
#include "Minte/Backend/VulkanBackend/VulkanMacros.hpp"

#include <SDL_vulkan.h>

#include <array>

namespace /* anonymous */
{
	/**
	 * Static initializer struct.
	 * These structs are used to initialize data that are to be initialized just once in the application.
	 */
	struct StaticInitializer final
	{
		/**
		 * Default constructor.
		 */
		StaticInitializer()
		{
			// Initialize volk.
			SDL_Init(SDL_INIT_VIDEO);
		}

		/**
		 * Destructor.
		 */
		~StaticInitializer()
		{
			// Quit SDL.
			SDL_Quit();
		}
	};
}

namespace minte
{
	namespace backend
	{
		VulkanWindow::VulkanWindow(const std::shared_ptr<VulkanInstance>& pInstance, std::string&& title, uint32_t width, uint32_t height)
			: Window(pInstance, std::move(title), width, height)
		{
			// Resolve the flags.
			uint32_t windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

			if (getWidth() == 0 || getHeight() == 0)
				windowFlags |= SDL_WINDOW_MAXIMIZED;
			else if (getWidth() == -1 || getHeight() == -1)
				windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

			// Create the window.
			m_pWindow = SDL_CreateWindow(getTitle().data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, getWidth(), getHeight(), windowFlags);

			// Set this class as user data.
			SDL_SetWindowData(m_pWindow, "this", this);

			// Check if we were able to create the window.
			if (!m_pWindow)
				throw BackendError("Failed to create the window!");

			// Make sure to show the window.
			SDL_ShowWindow(m_pWindow);

			// Refresh the window extent.
			refreshExtent();

			// Create the surface
			if (SDL_Vulkan_CreateSurface(m_pWindow, pInstance->getInstance(), &m_Surface) == SDL_FALSE)
				throw BackendError("Failed to create the window surface!");

			// Setup the rest.
			setupSwapchain();
			setupRenderPass();
			setupFramebuffer();
			setupSyncObjects();
		}

		VulkanWindow::~VulkanWindow()
		{
			clearSwapchain();

			const auto pInstance = getInstance()->as<VulkanInstance>();
			pInstance->getDeviceTable().vkDestroyRenderPass(pInstance->getLogicalDevice(), m_RenderPass, VK_NULL_HANDLE);
			pInstance->getDeviceTable().vkDestroyFramebuffer(pInstance->getLogicalDevice(), m_Framebuffer, VK_NULL_HANDLE);
			pInstance->getDeviceTable().vkDestroySemaphore(pInstance->getLogicalDevice(), m_RenderFinishedSemaphore, VK_NULL_HANDLE);
			pInstance->getDeviceTable().vkDestroySemaphore(pInstance->getLogicalDevice(), m_InFlightSemaphore, VK_NULL_HANDLE);

			vkDestroySurfaceKHR(pInstance->getInstance(), m_Surface, VK_NULL_HANDLE);
			SDL_DestroyWindow(m_pWindow);
		}

		void VulkanWindow::refreshExtent()
		{
			int32_t width = 0, height = 0;
			SDL_Vulkan_GetDrawableSize(m_pWindow, &width, &height);

			m_Width = width;
			m_Height = height;
		}

		void VulkanWindow::setupSwapchain()
		{
			const auto pInstance = getInstance()->as<VulkanInstance>();

			// Get the surface capabilities.
			VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
			MINTE_VK_ASSERT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pInstance->getPhysicalDevice(), m_Surface, &surfaceCapabilities), "Failed to get the surface capabilities!");

			// Get the surface formats.
			uint32_t formatCount = 0;
			MINTE_VK_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(pInstance->getPhysicalDevice(), m_Surface, &formatCount, VK_NULL_HANDLE), "Failed to get the surface format count!");

			if (formatCount == 0)
				throw BackendError("No suitable surface formats found!");

			std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
			MINTE_VK_ASSERT(vkGetPhysicalDeviceSurfaceFormatsKHR(pInstance->getPhysicalDevice(), m_Surface, &formatCount, surfaceFormats.data()), "Failed to get the surface formats!");

			// Get the present modes.
			uint32_t presentModeCount = 0;
			MINTE_VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(pInstance->getPhysicalDevice(), m_Surface, &presentModeCount, VK_NULL_HANDLE), "Failed to get the surface present mode count!");

			if (presentModeCount == 0)
				throw BackendError("No suitable present formats found!");

			std::vector<VkPresentModeKHR> presentModes(presentModeCount);
			MINTE_VK_ASSERT(vkGetPhysicalDeviceSurfacePresentModesKHR(pInstance->getPhysicalDevice(), m_Surface, &presentModeCount, presentModes.data()), "Failed to get the surface present modes!");

			// Check if we have the present mode we need.
			bool bPresentModeAvailable = false;
			VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			for (const auto availablePresentMode : presentModes)
			{
				if (availablePresentMode == presentMode)
				{
					bPresentModeAvailable = true;
					break;
				}
			}

			// If not available, let's just use the first one we got.
			if (!bPresentModeAvailable)
				presentMode = presentModes.front();

			// Resolve the surface composite.
			VkCompositeAlphaFlagBitsKHR surfaceComposite = static_cast<VkCompositeAlphaFlagBitsKHR>(surfaceCapabilities.supportedCompositeAlpha);
			surfaceComposite = (surfaceComposite & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
				? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
				: (surfaceComposite & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
				? VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR
				: (surfaceComposite & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
				? VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR
				: VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;

			// Get the best surface format.
			VkSurfaceFormatKHR surfaceFormat = surfaceFormats.front();
			for (const auto& availableFormat : surfaceFormats)
			{
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
					surfaceFormat = availableFormat;
			}

			m_SwapchainFormat = surfaceFormat.format;

			// Create the swap chain.
			VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
			swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchainCreateInfo.pNext = VK_NULL_HANDLE;
			swapchainCreateInfo.flags = 0;
			swapchainCreateInfo.surface = m_Surface;
			swapchainCreateInfo.minImageCount = 1;
			swapchainCreateInfo.imageFormat = m_SwapchainFormat;
			swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
			swapchainCreateInfo.imageExtent.width = m_Width;
			swapchainCreateInfo.imageExtent.height = m_Height;
			swapchainCreateInfo.imageArrayLayers = 1;
			swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainCreateInfo.queueFamilyIndexCount = 0;
			swapchainCreateInfo.pQueueFamilyIndices = VK_NULL_HANDLE;
			swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
			swapchainCreateInfo.compositeAlpha = surfaceComposite;
			swapchainCreateInfo.presentMode = presentMode;
			swapchainCreateInfo.clipped = VK_TRUE;
			swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

			// Resolve the queue families if the two queues are different.
			std::array<uint32_t, 2> queueFamilyindices = {
				pInstance->getGraphicsQueue().m_Family,
				pInstance->getTransferQueue().m_Family
			};

			if (queueFamilyindices[0] != queueFamilyindices[1])
			{
				swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				swapchainCreateInfo.queueFamilyIndexCount = 2;
				swapchainCreateInfo.pQueueFamilyIndices = queueFamilyindices.data();
			}

			MINTE_VK_ASSERT(pInstance->getDeviceTable().vkCreateSwapchainKHR(pInstance->getLogicalDevice(), &swapchainCreateInfo, VK_NULL_HANDLE, &m_Swapchain), "Failed to create the swapchain!");

			// Get the image.
			MINTE_VK_ASSERT(pInstance->getDeviceTable().vkGetSwapchainImagesKHR(pInstance->getLogicalDevice(), m_Swapchain, &swapchainCreateInfo.minImageCount, &m_SwapchainImage), "Failed to get the swapchain images!");

			// Create image views.
			VkImageViewCreateInfo viewCreateInfo = {};
			viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCreateInfo.pNext = VK_NULL_HANDLE;
			viewCreateInfo.flags = 0;
			viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewCreateInfo.format = m_SwapchainFormat;
			viewCreateInfo.components = {};
			viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewCreateInfo.subresourceRange.baseMipLevel = 0;
			viewCreateInfo.subresourceRange.levelCount = 1;
			viewCreateInfo.subresourceRange.baseArrayLayer = 0;
			viewCreateInfo.subresourceRange.layerCount = 1;

			MINTE_VK_ASSERT(pInstance->getDeviceTable().vkCreateImageView(pInstance->getLogicalDevice(), &viewCreateInfo, VK_NULL_HANDLE, &m_SwapchainImageView), "Failed to create the swapchain image view!");
		}

		void VulkanWindow::clearSwapchain()
		{
			const auto pInstance = getInstance()->as<VulkanInstance>();
			pInstance->getDeviceTable().vkDestroyImageView(pInstance->getLogicalDevice(), m_SwapchainImageView, VK_NULL_HANDLE);
			pInstance->getDeviceTable().vkDestroySwapchainKHR(pInstance->getLogicalDevice(), m_Swapchain, VK_NULL_HANDLE);
		}

		void VulkanWindow::setupRenderPass()
		{
			// Crate attachment descriptions.
			VkAttachmentDescription attachmentDescription = {};
			attachmentDescription.flags = 0;
			attachmentDescription.format = m_SwapchainFormat;
			attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	// TODO: Play around with this so that we don't do anything stupid with the dependency copy.
			attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			// Create the subpass dependencies.
			std::array<VkSubpassDependency, 2> subpassDependencies = {};
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
			VkAttachmentReference colorAttachmentReference = {};
			colorAttachmentReference.attachment = 0;
			colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpassDescription = {};
			subpassDescription.flags = 0;
			subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescription.inputAttachmentCount = 0;
			subpassDescription.pInputAttachments = VK_NULL_HANDLE;
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = &colorAttachmentReference;
			subpassDescription.pResolveAttachments = VK_NULL_HANDLE;
			subpassDescription.pDepthStencilAttachment = VK_NULL_HANDLE;
			subpassDescription.preserveAttachmentCount = 0;
			subpassDescription.pPreserveAttachments = VK_NULL_HANDLE;

			// Create the render target.
			VkRenderPassCreateInfo renderPassCreateInfo = {};
			renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassCreateInfo.pNext = VK_NULL_HANDLE;
			renderPassCreateInfo.flags = 0;
			renderPassCreateInfo.attachmentCount = 1;
			renderPassCreateInfo.pAttachments = &attachmentDescription;
			renderPassCreateInfo.subpassCount = 1;
			renderPassCreateInfo.pSubpasses = &subpassDescription;
			renderPassCreateInfo.dependencyCount = 2;
			renderPassCreateInfo.pDependencies = subpassDependencies.data();

			const auto pInstance = getInstance()->as<VulkanInstance>();
			MINTE_VK_ASSERT(pInstance->getDeviceTable().vkCreateRenderPass(pInstance->getLogicalDevice(), &renderPassCreateInfo, VK_NULL_HANDLE, &m_RenderPass), "Failed to create render pass!");
		}

		void VulkanWindow::setupFramebuffer()
		{
			VkFramebufferCreateInfo frameBufferCreateInfo = {};
			frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferCreateInfo.pNext = VK_NULL_HANDLE;
			frameBufferCreateInfo.flags = 0;
			frameBufferCreateInfo.renderPass = m_RenderPass;
			frameBufferCreateInfo.attachmentCount = 1;
			frameBufferCreateInfo.width = m_Width;
			frameBufferCreateInfo.height = m_Height;
			frameBufferCreateInfo.layers = 1;
			frameBufferCreateInfo.pAttachments = &m_SwapchainImageView;

			const auto pInstance = getInstance()->as<VulkanInstance>();
			MINTE_VK_ASSERT(pInstance->getDeviceTable().vkCreateFramebuffer(pInstance->getLogicalDevice(), &frameBufferCreateInfo, VK_NULL_HANDLE, &m_Framebuffer), "Failed to create the frame buffer!");
		}

		void VulkanWindow::setupSyncObjects()
		{
			VkSemaphoreCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			createInfo.pNext = VK_NULL_HANDLE;
			createInfo.flags = 0;

			const auto pInstance = getInstance()->as<VulkanInstance>();
			MINTE_VK_ASSERT(pInstance->getDeviceTable().vkCreateSemaphore(pInstance->getLogicalDevice(), &createInfo, VK_NULL_HANDLE, &m_InFlightSemaphore), "Failed to create the in flight semaphore!");
			MINTE_VK_ASSERT(pInstance->getDeviceTable().vkCreateSemaphore(pInstance->getLogicalDevice(), &createInfo, VK_NULL_HANDLE, &m_RenderFinishedSemaphore), "Failed to create the render finished semaphore!");
		}
	}
}