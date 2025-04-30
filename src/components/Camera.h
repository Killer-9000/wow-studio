#pragma once

#include "BaseComponent.h"

struct Camera : public BaseComponent
{

	static Camera* s_mainCamera = nullptr;
};
