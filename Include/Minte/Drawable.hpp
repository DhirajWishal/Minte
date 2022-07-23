// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "MinteObject.hpp"

namespace minte
{
	/**
	 * Drawable class.
	 * This contains information to draw something to a layer.
	 */
	class Drawable : public MinteObject
	{
	public:
		/**
		 * Default constructor.
		 */
		Drawable() = default;

		/**
		 * Explicit constructor.
		 *
		 * @param parent The parent to which the object belongs to.
		 */
		explicit Drawable(Minte parent) : MinteObject(parent) {}

		/**
		 * Default virtual destructor.
		 */
		virtual ~Drawable() = default;
	};
}