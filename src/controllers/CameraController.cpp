
#include "CameraController.h"
#include "components/common/Transform.h"
#include "core/window/input/InputHandler.h"


using namespace fungine;
using namespace components;
using namespace entities;
using namespace core;


CameraController::CameraController(Entity* cameraToControl)
{
	_cameraEntity = cameraToControl;
}

void CameraController::update()
{
	float speed = 0.01f;
	float rotationSpeed = 0.003f;

	float forward = 0.0f;
	float sideways = 0.0f;
	float up = 0.0f;
	
	if (InputHandler::is_key_down(FUNGINE_KEY_W)) forward = -speed;
	if (InputHandler::is_key_down(FUNGINE_KEY_S)) forward = speed;
	
	if (InputHandler::is_key_down(FUNGINE_KEY_A)) sideways = -speed;
	if (InputHandler::is_key_down(FUNGINE_KEY_D)) sideways = speed;

	if (InputHandler::is_key_down(FUNGINE_KEY_SPACE)) up = speed;
	if (InputHandler::is_key_down(FUNGINE_KEY_LEFT_CTRL)) up = -speed;

	
	Transform* t = _cameraEntity->getComponent<Transform>().get();
	mml::Vector3 pos = t->getPosition() + t->forward() * forward + t->up() * up + t->right() * sideways;
	t->setPosition(pos);

	if (InputHandler::is_mouse_down(0))
	{
		_yaw -= InputHandler::get_mouse_dx() * rotationSpeed;

		float prevPitch = _pitch;
		_pitch += InputHandler::get_mouse_dy() * rotationSpeed;
		if (std::cos(_pitch) < 0)
			_pitch = prevPitch;

		mml::Quaternion rot = mml::Quaternion({ 0,1,0 }, _yaw) * mml::Quaternion({ 1,0,0 }, _pitch);
		t->setRotation(rot);
	}
	
}