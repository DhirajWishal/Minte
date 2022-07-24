// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "ImageBuffer.hpp"

namespace minte
{
	namespace backend
	{
		/**
		 * Anti aliasing used by the render target.
		 */
		enum class AntiAliasing : uint8_t
		{
			X1,
			X2,
			X4,
			X8,
			X16,
			X32,
			X64
		};

		/**
		 * Render Target.
		 * This class renders a layer and it's elements and returns the resulting image to the user.
		 *
		 * The render target contains 3 buffer, the color, entity and depth buffers.
		 * * Color buffer is the actual rendered output.
		 * * Entity buffer contains the entity IDs of all the drawn elements, and can be used for mouse picking.
		 * * The depth buffer contains, well, the depth information.
		 *
		 * The derived class is expected to initialize these members using the three protected functions set*Buffer(). And should contain the
		 * respective data at the end of the draw call. And the buffer size(s) should be equal to (width * height * pixel_size).
		 */
		class RenderTarget : public InstanceBoundObject
		{
		public:
			/**
			 * Default constructor.
			 */
			constexpr RenderTarget() = default;

			/**
			 * Explicit constructor.
			 *
			 * @param pInstance The instance pointer.
			 * @param width The width of the render target.
			 * @param height The height of the render target.
			 * @param antiAliasing The anti aliasing to use. Default is x1.
			 */
			explicit RenderTarget(const std::shared_ptr<Instance>& pInstance, uint32_t width, uint32_t height, AntiAliasing antiAliasing = AntiAliasing::X1)
				: InstanceBoundObject(pInstance), m_Width(width), m_Height(height), m_AntiAliasing(antiAliasing) {}

			/**
			 * Default virtual destructor.
			 */
			virtual ~RenderTarget() = default;

			/**
			 * Draw all the entities that are bound to the render target.
			 */
			virtual void draw() = 0;

			/**
			 * Get the width of the render target.
			 *
			 * @return The width.
			 */
			[[nodiscard]] uint32_t getWidth() const { return m_Width; }

			/**
			 * Get the height of the render target.
			 *
			 * @return The height.
			 */
			[[nodiscard]] uint32_t getHeight() const { return m_Height; }

			/**
			 * Get the anti-aliasing value.
			 *
			 * @return The value.
			 */
			[[nodiscard]] AntiAliasing getAntiAliasing() const { return m_AntiAliasing; }

			/**
			 * Get the color buffer.
			 *
			 * @return The color buffer.
			 */
			[[nodiscard]] const ImageBuffer* getColorBuffer() const { return m_pColorBuffer.get(); }

			/**
			 * Get the entity buffer.
			 *
			 * @return The entity buffer.
			 */
			[[nodiscard]] const ImageBuffer* getEntityBuffer() const { return m_pEntityBuffer.get(); }

			/**
			 * Get the depth buffer.
			 *
			 * @return The depth buffer.
			 */
			[[nodiscard]] const ImageBuffer* getDepthBuffer() const { return m_pDepthBuffer.get(); }

		protected:
			/**
			 * Set the entity buffer.
			 * This is required to be set by the derived class.
			 *
			 * @param pBuffer The buffer to set.
			 */
			void setColorBuffer(std::unique_ptr<ImageBuffer>&& pBuffer) { m_pColorBuffer = std::move(pBuffer); }

			/**
			 * Set the entity buffer.
			 * This is required to be set by the derived class.
			 *
			 * @param pBuffer The buffer to set.
			 */
			void setEntityBuffer(std::unique_ptr<ImageBuffer>&& pBuffer) { m_pEntityBuffer = std::move(pBuffer); }

			/**
			 * Set the entity buffer.
			 * This is required to be set by the derived class.
			 *
			 * @param pBuffer The buffer to set.
			 */
			void setDepthBuffer(std::unique_ptr<ImageBuffer>&& pBuffer) { m_pDepthBuffer = std::move(pBuffer); }

		private:
			std::unique_ptr<ImageBuffer> m_pColorBuffer = nullptr;
			std::unique_ptr<ImageBuffer> m_pEntityBuffer = nullptr;
			std::unique_ptr<ImageBuffer> m_pDepthBuffer = nullptr;

			uint32_t m_Width = 0;
			uint32_t m_Height = 0;

			AntiAliasing m_AntiAliasing = AntiAliasing::X1;
		};
	}
}