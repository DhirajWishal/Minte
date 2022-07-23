// Copyright (c) 2022 Dhiraj Wishal

#include "Minte/Layer.hpp"
#include "Minte/FrontendError.hpp"

#include "VulkanBackend/VulkanRenderTarget.hpp"

namespace minte
{
	Layer::Layer(Minte parent, uint32_t width, uint32_t height)
		: MinteObject(parent)
		, m_Rectangle(Point2D_UI32(0), Point2D_UI32(width, height))
		, m_pRenderTarget(std::make_unique<backend::VulkanRenderTarget>(std::static_pointer_cast<backend::VulkanInstance>(parent.getInstance()), width, height))
	{
	}

	void Layer::update()
	{
		// We need to update only if the render target is valid.
		if (m_pRenderTarget->isValid())
		{
			m_pRenderTarget->draw();
		}
	}

}