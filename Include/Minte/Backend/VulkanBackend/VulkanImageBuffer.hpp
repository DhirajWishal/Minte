// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "../ImageBuffer.hpp"
#include "VulkanInstance.hpp"

namespace minte
{
	namespace backend
	{
		/**
		 * Vulkan image buffer class.
		 */
		class VulkanImageBuffer final : public ImageBuffer
		{
		public:
			/**
			 * Default constructor.
			 */
			constexpr VulkanImageBuffer() = default;

			/**
			 * Explicit constructor.
			 *
			 * @param pInstance The instance pointer.
			 * @param size The size of the buffer.
			 */
			explicit VulkanImageBuffer(const std::shared_ptr<VulkanInstance>& pInstance, uint64_t size);

			/**
			 * Destructor.
			 */
			~VulkanImageBuffer() override;

			/**
			 * Map the buffer memory to the local address space.
			 *
			 * @return The accessed bytes.
			 */
			[[nodiscard]] std::byte* mapMemory() override;

			/**
			 * Unmap the mapped memory.
			 */
			void unmapMemory() override;

			/**
			 * Get the buffer.
			 *
			 * @return The buffer.
			 */
			[[nodiscard]] VkBuffer getBuffer() const { return m_Buffer; }

		private:
			VkBuffer m_Buffer = VK_NULL_HANDLE;
			VmaAllocation m_Allocation = nullptr;
		};
	}
}
