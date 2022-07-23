// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include <memory>

namespace minte
{
	namespace backend
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

			/**
			 * Get this object casted to another type.
			 *
			 * @tparam Type The type to cast to.
			 * @return The casted type pointer.
			 */
			template<class Type>
			[[nodiscard]] Type* as() { return static_cast<Type*>(this); }

			/**
			 * Get this object casted to another type.
			 *
			 * @tparam Type The type to cast to.
			 * @return The casted type pointer.
			 */
			template<class Type>
			[[nodiscard]] const Type* as() const { return static_cast<const Type*>(this); }
		};
	}
}