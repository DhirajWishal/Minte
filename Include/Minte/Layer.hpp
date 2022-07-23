// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "InstanceBoundObject.hpp"
#include "DataTypes.hpp"

namespace minte
{
	/**
	 * Layer class.
	 * This class contains a single image which can be retrieved after drawing.
	 */
	class Layer : public InstanceBoundObject
	{
	public:
		/**
		 * Default constructor.
		 */
		constexpr Layer() = default;

		/**
		 * Explicit constructor.
		 *
		 * @param pInstance The instance pointer.
		 * @param width The width of the rectangle.
		 * @param height The height of the rectangle.
		 */
		explicit Layer(const std::shared_ptr<Instance>& pInstance, uint32_t width, uint32_t height) : InstanceBoundObject(pInstance), m_Rectangle(Point2D_UI32(0), Point2D_UI32(width, height)) {}

		/**
		 * Create a new element to this layer.
		 * If the element is derived from InstanceBoundObject, the instance pointer will be provided.
		 *
		 * @tparam Element The element type.
		 * @tparam Args The constructor arguments.
		 * @param arguments The arguments required by the Element's constructor.
		 * @return The created element.
		 */
		template<class Element, class... Args>
		[[nodiscard]] Element createElement(Args&&... arguments)
		{
			if constexpr (std::is_base_of_v<InstanceBoundObject, Element>)
				return Element(getInstancePointer(), std::forward<Args>(arguments)...);

			else
				return Element(std::forward<Args>(arguments)...);
		}

	private:
		Rectangle2D m_Rectangle = {};
	};
}