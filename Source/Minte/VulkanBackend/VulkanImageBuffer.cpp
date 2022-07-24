// Copyright (c) 2022 Dhiraj Wishal

#include "Minte/Backend/VulkanBackend/VulkanImageBuffer.hpp"
#include "Minte/Backend/VulkanBackend/VulkanMacros.hpp"

namespace minte
{
	namespace backend
	{
		VulkanImageBuffer::VulkanImageBuffer(const std::shared_ptr<VulkanInstance>& pInstance, uint64_t size)
			: ImageBuffer(pInstance, size)
		{
			VkBufferCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			createInfo.pNext = VK_NULL_HANDLE;
			createInfo.flags = 0;
			createInfo.size = size;
			createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = VK_NULL_HANDLE;

			VmaAllocationCreateInfo allocationCreateInfo = {};
			allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;

			MINTE_VK_ASSERT(vmaCreateBuffer(getInstance()->as<VulkanInstance>()->getAllocator(), &createInfo, &allocationCreateInfo, &m_Buffer, &m_Allocation, VK_NULL_HANDLE), "Failed to create the buffer!");
		}

		VulkanImageBuffer::~VulkanImageBuffer()
		{
			vmaDestroyBuffer(getInstance()->as<VulkanInstance>()->getAllocator(), m_Buffer, m_Allocation);
		}

		std::byte* VulkanImageBuffer::mapMemory()
		{
			std::byte* dataPointer = nullptr;
			MINTE_VK_ASSERT(vmaMapMemory(getInstance()->as<VulkanInstance>()->getAllocator(), m_Allocation, reinterpret_cast<void**>(&dataPointer)), "Failed to map the buffer memory!");

			m_IsMapped = true;
			return dataPointer;
		}

		void VulkanImageBuffer::unmapMemory()
		{
			// Return if we haven't mapped the memory.
			if (!isMapped())
				return;

			vmaUnmapMemory(getInstance()->as<VulkanInstance>()->getAllocator(), m_Allocation);
			m_IsMapped = false;
		}
	}
}