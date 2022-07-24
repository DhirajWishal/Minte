// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "InstanceBoundObject.hpp"

namespace minte
{
	namespace backend
	{
		/**
		 * Image buffer class.
		 * This is used to submit image data from the backend to the frontend and vice versa.
		 */
		class ImageBuffer : public InstanceBoundObject
		{
		public:
			/**
			 * Default constructor.
			 */
			constexpr ImageBuffer() = default;

			/**
			 * Explicit constructor.
			 *
			 * @param pInstance The instance pointer.
			 * @param size The size of the buffer.
			 */
			explicit ImageBuffer(const std::shared_ptr<Instance>& pInstance, uint64_t size) : InstanceBoundObject(pInstance), m_Size(size) {}

			/**
			 * Default virtual destructor.
			 */
			virtual ~ImageBuffer() = default;

			/**
			 * Map the buffer memory to the local address space.
			 *
			 * @return The accessed bytes.
			 */
			[[nodiscard]] virtual std::byte* mapMemory() = 0;

			/**
			 * Unmap the mapped memory.
			 */
			virtual void unmapMemory() = 0;

			/**
			 * Get the size of the buffer.
			 *
			 * @return The size.
			 */
			[[nodiscard]] uint64_t getSize() const { return m_Size; }

			/**
			 * Check if the buffer is mapped or not.
			 *
			 * @return Whether it's mapped or not.
			 */
			[[nodiscard]] bool isMapped() const { return m_IsMapped; }

		protected:
			uint64_t m_Size = 0;
			bool m_IsMapped = false;
		};
	}
}