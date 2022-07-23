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
	HeadsUpDisplay() = default;

	/**
	 * Explicit constructor.
	 *
	 * @param parent The parent to which the object belongs to.
	 */
	explicit HeadsUpDisplay(minte::Minte parent);
};