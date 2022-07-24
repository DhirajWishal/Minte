// Copyright (c) 2022 Dhiraj Wishal

#include "Minte/Layer.hpp"
#include "Minte/FrontendError.hpp"

namespace minte
{
	Layer::Layer(Minte parent, std::unique_ptr<backend::RenderTarget>&& pRenderTarget)
		: MinteObject(parent)
		, m_pRenderTarget(std::move(pRenderTarget))
	{
	}

	LayerOutput Layer::update()
	{
		LayerOutput output;

		// We need to update only if the render target is valid.
		if (m_pRenderTarget->isValid())
		{
			m_pRenderTarget->draw();

			// Get the output images.
			output.m_pColorBuffer = m_pRenderTarget->getColorBuffer();
			output.m_pEntityBuffer = m_pRenderTarget->getEntityBuffer();
			output.m_pDepthBuffer = m_pRenderTarget->getDepthBuffer();
		}

		return output;
	}

}