// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "Backend/Instance.hpp"

namespace minte
{
	/**
	 * Minte class.
	 * This is the main class of the whole library and contains the rendering backend.
	 */
	class Minte final
	{
	public:
		/**
		 * Default constructor.
		 */
		Minte();

		/**
		 * Default destructor.
		 */
		~Minte() = default;

		/**
		 * Get the instance object.
		 *
		 * @return The instance pointer.
		 */
		[[nodsicard]] std::shared_ptr<backend::Instance> getInstance() { return m_pInstance; }

		/**
		 * Get the instance object.
		 *
		 * @return The instance pointer.
		 */
		[[nodsicard]] std::shared_ptr<const backend::Instance> getInstance() const { return m_pInstance; }

	private:
		std::shared_ptr<backend::Instance> m_pInstance = nullptr;
	};
}