// Copyright (c) 2022 Dhiraj Wishal

#pragma once

#include "Backend/BackendError.hpp"

#define MINTE_VK_ASSERT(exp, message) if(exp != VK_SUCCESS) throw ::minte::backend::BackendError(message)