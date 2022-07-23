// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "InstanceBoundObject.hpp"

#include <string>

namespace minte
{
	namespace backend
	{
		/**
		 * Window class.
		 * This can be used to render content to the screen.
		 */
		class Window : public InstanceBoundObject
		{
		public:
			/**
			 * Default constructor.
			 */
			constexpr Window() = default;

			/**
			 * Explicit constructor.
			 *
			 * @param pInstance The instance to which the window is bound to.
			 * @param title The title of the window.
			 * @param width The window width.
			 * @param height The window height.
			 */
			explicit Window(const std::shared_ptr<Instance>& pInstance, std::string&& title, uint32_t width, uint32_t height)
				: InstanceBoundObject(pInstance), m_Title(std::move(title)), m_Width(width), m_Height(height) {}

			/**
			 * Virtual default destructor.
			 */
			virtual ~Window() = default;

			/**
			 * Get the title of the window.
			 *
			 * @return The title.
			 */
			[[nodiscard]] std::string_view getTitle() const { return m_Title; }

			/**
			 * Get the width of the window.
			 *
			 * @return The value.
			 */
			[[nodiscard]] uint32_t getWidth() const { return m_Width; }

			/**
			 * Get the height of the window.
			 *
			 * @return The value.
			 */
			[[nodiscard]] uint32_t getHeight() const { return m_Height; }

		protected:
			std::string m_Title;
			uint32_t m_Width = 0;
			uint32_t m_Height = 0;
		};
	}
}