#pragma once

#include "entities/Entity.h"

class CameraController
{
private:

	fungine::entities::Entity* _cameraEntity = nullptr;

	float _pitch = 0.0f;
	float _yaw = 0.0f;

public:

	CameraController(fungine::entities::Entity* cameraToControl);
	void update();
};