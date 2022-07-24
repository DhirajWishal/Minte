// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "MinteObject.hpp"
#include "DataTypes.hpp"

#include "Backend/RenderTarget.hpp"

namespace minte
{
	/**
	 * Layer output structure.
	 * This contains the layer's rendered output.
	 */
	struct LayerOutput final
	{
		const backend::ImageBuffer* m_pColorBuffer = nullptr;
		const backend::ImageBuffer* m_pEntityBuffer = nullptr;
		const backend::ImageBuffer* m_pDepthBuffer = nullptr;
	};

	/**
	 * Layer class.
	 * This class contains a single image which can be retrieved after drawing.
	 */
	class Layer : public MinteObject
	{
	public:
		/**
		 * Default constructor.
		 */
		Layer() = default;

		/**
		 * Explicit constructor.
		 *
		 * @param parent The parent of this class.
		 * @param pRenderTarget The render target to render the layer with.
		 */
		explicit Layer(Minte parent, std::unique_ptr<backend::RenderTarget>&& pRenderTarget);

		/**
		 * Default virtual destructor.
		 */
		virtual ~Layer() = default;

		/**
		 * Create a new element to this layer.
		 * If the element is derived from MinteObject, the parent will be provided.
		 *
		 * @tparam Element The element type.
		 * @tparam Arguments The constructor arguments.
		 * @param arguments The arguments required by the Element's constructor.
		 * @return The created element.
		 */
		template<class Element, class... Arguments>
		[[nodiscard]] Element createElement(Arguments&&... arguments)
		{
			if constexpr (std::is_base_of_v<MinteObject, Element>)
				return Element(getParent(), std::forward<Arguments>(arguments)...);

			else
				return Element(std::forward<Arguments>(arguments)...);
		}

		/**
		 * Update the layer.
		 * This will first draw all the UI elements and then handle inputs.
		 *
		 * @return The rendered images.
		 */
		[[nodiscard]] LayerOutput update();

	private:
		std::unique_ptr<backend::RenderTarget> m_pRenderTarget = nullptr;
	};
}