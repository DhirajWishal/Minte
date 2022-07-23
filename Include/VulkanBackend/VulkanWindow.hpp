// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "Backend/Window.hpp"
#include "VulkanInstance.hpp"

#include <SDL.h>

namespace minte
{
	namespace backend
	{
		/**
		 * Vulkan window class.
		 */
		class VulkanWindow final : public Window
		{
		public:
			/**
			 * Default constructor.
			 */
			constexpr VulkanWindow() = default;

			/**
			 * Explicit constructor.
			 *
			 * @param pInstance The instance to which the window is bound to.
			 * @param title The title of the window.
			 * @param width The window width.
			 * @param height The window height.
			 */
			explicit VulkanWindow(const std::shared_ptr<VulkanInstance>& pInstance, std::string&& title, uint32_t width, uint32_t height);

			/**
			 * Destructor.
			 */
			~VulkanWindow() override;

		private:
			/**
			 * Refresh the window extent.
			 */
			void refreshExtent();

			/**
			 * Setup the swapchain.
			 */
			void setupSwapchain();

			/**
			 * Clear the swapchain data.
			 */
			void clearSwapchain();

			/**
			 * Setup the render pass.
			 */
			void setupRenderPass();

			/**
			 * Setup the frame buffer.
			 */
			void setupFramebuffer();

			/**
			 * Setup the sync objects.
			 */
			void setupSyncObjects();

		private:
			SDL_Window* m_pWindow = nullptr;

			VkImage m_SwapchainImage = VK_NULL_HANDLE;
			VkImageView m_SwapchainImageView = VK_NULL_HANDLE;

			VkSemaphore m_InFlightSemaphore = VK_NULL_HANDLE;
			VkSemaphore m_RenderFinishedSemaphore = VK_NULL_HANDLE;

			VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
			VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

			VkRenderPass m_RenderPass = VK_NULL_HANDLE;
			VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;

			VkFormat m_SwapchainFormat = VK_FORMAT_UNDEFINED;
		};
	}
}