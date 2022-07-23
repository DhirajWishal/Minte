// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "Minte/Layer.hpp"

/**
 * The HUD contains some basic elements.
 */
class HeadsUpDisplay final : public minte::Layer
{
public:
	/**
	 * Default constructor.
	 */
	constexpr HeadsUpDisplay() = default;

	/**
	 * Explicit constructor.
	 *
	 * @param pInstance The instance pointer.
	 */
	explicit HeadsUpDisplay(const std::shared_ptr<minte::Instance>& pInstance);
};