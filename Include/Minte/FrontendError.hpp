// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include <stdexcept>

namespace minte
{
	/**
	 * Frontend error class.
	 * This class is thrown if the frontend is encountered with any errors.
	 */
	class FrontendError final : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error;
	};
}