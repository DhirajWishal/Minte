// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "InstanceBoundObject.hpp"

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

		private:
			uint32_t m_Width = 0;
			uint32_t m_Height = 0;

			const AntiAliasing m_AntiAliasing = AntiAliasing::X1;
		};
	}
}