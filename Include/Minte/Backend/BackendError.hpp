// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include <stdexcept>

namespace minte
{
	namespace backend
	{
		/**
		 * Backend error class.
		 * This class is thrown if the backend encountered any errors.
		 */
		class BackendError final : public std::runtime_error
		{
		public:
			using std::runtime_error::runtime_error;
		};
	}
}