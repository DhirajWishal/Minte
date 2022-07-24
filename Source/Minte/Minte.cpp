// Copyright (c) 2022 Dhiraj Wishal

#include "Minte/Minte.hpp"

namespace minte
{
	Minte::Minte(const std::shared_ptr<backend::Instance>& pInstance)
		: m_pInstance(pInstance)
	{
	}
}