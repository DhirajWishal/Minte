// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "../InstanceBoundObject.hpp"

namespace minte
{
	namespace backend
	{
		/**
		 * Render Target.
		 * This class renders a layer and it's elements and returns the resulting image to the user.
		 */
		class RenderTarget : public InstanceBoundObject
		{
		public:
			/**
			 * Explicit constructor.
			 *
			 * @param pInstance The instance pointer.
			 * @param width The width of the render target.
			 * @param height The height of the render target.
			 */
			explicit RenderTarget(const std::shared_ptr<Instance>& pInstance, uint32_t width, uint32_t height) : InstanceBoundObject(pInstance), m_Width(width), m_Height(height) {}

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

		private:
			uint32_t m_Width = 0;
			uint32_t m_Height = 0;
		};
	}
}