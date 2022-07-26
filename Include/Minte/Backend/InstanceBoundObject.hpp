// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "Instance.hpp"

namespace minte
{
	namespace backend
	{
		/**
		 * Instance bound object class.
		 * This stores a single instance pointer.
		 *
		 * Note that if the internal pointer is nullptr, then the object is considered as invalid.
		 */
		class InstanceBoundObject
		{
		public:
			/**
			 * Default constructor.
			 */
			constexpr InstanceBoundObject() = default;

			/**
			 * Explicit constructor.
			 *
			 * @param pInstance The instance pointer.
			 */
			explicit InstanceBoundObject(const std::shared_ptr<Instance>& pInstance) : m_pInstance(pInstance) {}

			/**
			 * Default virtual destructor.
			 */
			virtual ~InstanceBoundObject() = default;

			/**
			 * Check if the object is valid or not.
			 *
			 * @return true if valid, false if not.
			 */
			[[nodsicard]] bool isValid() const noexcept { return m_pInstance != nullptr; }

			/**
			 * Get this object casted to another type.
			 *
			 * @tparam Type The type to cast to.
			 * @return The casted type pointer.
			 */
			template<class Type>
			[[nodsicard]] Type* as() { return static_cast<Type*>(this); }

			/**
			 * Get this object casted to another type.
			 *
			 * @tparam Type The type to cast to.
			 * @return The casted type pointer.
			 */
			template<class Type>
			[[nodsicard]] const Type* as() const { return static_cast<const Type*>(this); }

			/**
			 * Get the raw instance pointer.
			 *
			 * @return The instance pointer.
			 */
			[[nodsicard]] Instance* getInstance() { return m_pInstance.get(); }

			/**
			 * Get the raw instance pointer.
			 *
			 * @return The instance pointer.
			 */
			[[nodsicard]] const Instance* getInstance() const { return m_pInstance.get(); }

			/**
			 * Get the shared instance pointer.
			 *
			 * @return The instance pointer.
			 */
			[[nodsicard]] std::shared_ptr<Instance> getInstancePointer() { return m_pInstance; }

			/**
			 * Get the shared instance pointer.
			 *
			 * @return The instance pointer.
			 */
			[[nodsicard]] std::shared_ptr<const Instance> getInstancePointer() const { return m_pInstance; }

		private:
			std::shared_ptr<Instance> m_pInstance = nullptr;
		};
	}
}