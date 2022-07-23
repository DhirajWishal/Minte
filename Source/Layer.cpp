// Copyright (c) 2022 Dhiraj Wishal

#include "Minte/Layer.hpp"
#include "Minte/FrontendError.hpp"

#include "VulkanBackend/VulkanRenderTarget.hpp"

namespace minte
{
	Layer::Layer(const std::shared_ptr<Instance>& pInstance, uint32_t width, uint32_t height)
		: InstanceBoundObject(pInstance)
		, m_Rectangle(Point2D_UI32(0), Point2D_UI32(width, height))
		, m_pRenderTarget(std::make_unique<VulkanRenderTarget>(std::static_pointer_cast<VulkanInstance>(pInstance), width, height))
	{
	}

	void Layer::update()
	{
		// We need to update only if the frontend is valid.
		if (isValid())
		{
			m_pRenderTarget->draw();
		}
	}

}