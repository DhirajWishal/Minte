// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include <memory>

namespace minte
{
	/**
	 * Instance class.
	 */
	class Instance
	{
	public:
		/**
		 * Default constructor.
		 */
		constexpr Instance() = default;

		/**
		 * Virtual default destructor.
		 */
		virtual ~Instance() = default;
	};

	/**
	 * Create a new instance pointer.
	 *
	 * @return The instance pointer.
	 */
	[[nodiscard]] std::shared_ptr<Instance> CreateInstance();
}