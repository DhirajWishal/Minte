// Copyright (c) 2022 Dhiraj Wishal

#include "Minte/Layer.hpp"

#include "VulkanBackend/VulkanRenderTarget.hpp"

namespace minte
{
	Layer::Layer(const std::shared_ptr<Instance>& pInstance, uint32_t width, uint32_t height)
		: InstanceBoundObject(pInstance)
		, m_Rectangle(Point2D_UI32(0), Point2D_UI32(width, height))
		, m_pRenderTarget(std::make_unique<VulkanRenderTarget>(std::static_pointer_cast<VulkanInstance>(pInstance), width, height))
	{
	}
}