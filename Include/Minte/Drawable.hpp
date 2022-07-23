// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "InstanceBoundObject.hpp"

namespace minte
{
	/**
	 * Drawable class.
	 * This contains information to draw something to a layer.
	 */
	class Drawable : public InstanceBoundObject
	{
	public:
		/**
		 * Default constructor.
		 */
		constexpr Drawable() = default;

		/**
		 * Explicit constructor.
		 *
		 * @param pInstance The instance pointer.
		 */
		explicit Drawable(const std::shared_ptr<Instance>& pInstance) : InstanceBoundObject(pInstance) {}

		/**
		 * Default virtual destructor.
		 */
		virtual ~Drawable() = default;
	};
}