// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "Minte.hpp"

namespace minte
{
	/**
	 * Minte object class.
	 * This is the base class for all the GUI classes.
	 */
	class MinteObject
	{
	public:
		/**
		 * Default constructor.
		 */
		MinteObject() = default;

		/**
		 * Explicit constructor.
		 *
		 * @param parent The parent class.
		 */
		explicit MinteObject(const Minte& parent) : m_Parent(parent) {}

		/**
		 * Get the parent object.
		 *
		 * @return The parent.
		 */
		[[nodiscard]] Minte& getParent() { return m_Parent; }

		/**
		 * Get the parent object.
		 *
		 * @return The parent.
		 */
		[[nodiscard]] const Minte& getParent() const { return m_Parent; }

	private:
		Minte m_Parent;
	};
}