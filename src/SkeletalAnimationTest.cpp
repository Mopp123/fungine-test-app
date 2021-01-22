#include "TestProgram.h"
#ifdef MAIN_FUNC__SkeletalAnimationTest

#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include "core/Program.h"
#include "utils/myMathLib/MyMathLib.h"
#include "core/Common.h"
#include "core/Debug.h"
#include "utils/Time.h"

#include "graphics/Buffers.h"
#include "graphics/shaders/ShaderStage.h"
#include "graphics/shaders/ShaderUniforms.h"
#include "graphics/Texture.h"
#include "graphics/Framebuffer.h"

#include "components/rendering/Mesh.h"
#include "components/rendering/Material.h"
#include "components/common/Transform.h"
#include "components/rendering/Camera.h"
#include "components/rendering/lighting/Lights.h"
#include "components/rendering/renderers/Renderer.h"
#include "components/rendering/renderers/TerrainRenderer.h"
#include "components/rendering/renderers/NatureRenderer.h"

#include "utils/modelLoading/ModelLoading.h"

#include "entities/Entity.h"
#include "entities/terrain/Terrain.h"
#include "entities/commonEntitiesLib/CommonEntityFactory.h"
#include "entities/commonEntitiesLib/shapes3D/CommonShapes.h"

#include "controllers/CameraController.h"

#include <stdio.h>
#include <cmath>

/*
	* Uses "Fungine" as static lib
		(*but fungine uses assimp as dll so we need to have that..)
*/



/*
	"Comment tags"( all of them starts with '*->' ):

	*->TEMP = very temporary testing thing that should be quickly removed/changed to more final form.
	*->TODO = ..to doo

	NEXT UP:

		* remove framebuffer's depth texture attachment's border coloring hack!
		* figure out why dir light view matrix is inverted..
		* improve rendering system (shadowmaprenderpass things..)
		* shadowmap pcf
		* change constant material specular properties from shaders to uniforms
*/

using namespace fungine;
using namespace core;
using namespace components;
using namespace rendering;
using namespace entities;
using namespace graphics;
using namespace entities;


int main(int argc, const char** argv)
{

	unsigned int windowWidth = 1024;
	unsigned int windowHeight = 768;
	unsigned int windowFullscreen = 0;
	unsigned int shadowmapWidth = 1024;
	unsigned int vSync = 0;

	unsigned int instanceCount_trees = 1000;
	unsigned int instanceCount_grass = 100000;

	Program program("FunGINe engine demo", windowWidth, windowHeight, windowFullscreen == 0 ? false : true, vSync);

	// Create perspective projection matrix
	const float aspectRatio = (float)(Window::get_width()) / (float)(Window::get_height());
	const float fov = mml::to_radians(70.0f);
	// Create camera entity
	const float camStartPitch = mml::to_radians(-17.0f);
	const float camStartYaw = mml::to_radians(-135.0f);
	mml::Quaternion camStartRot = mml::Quaternion({ 0,1,0 }, camStartYaw) * mml::Quaternion({ 1,0,0 }, camStartPitch);
	Entity* cameraEntity = commonEntityFactory::create_entity__Camera({ 35,145,35 }, camStartRot);
	cameraEntity->getComponent<Camera>()->setPerspectiveProjection(fov, aspectRatio, 0.1f, 1000.0f);
	CameraController camController(cameraEntity, camStartPitch, camStartYaw);

	// Create directional light entity
	float dirLight_pitch = mml::to_radians(45.0f);
	float dirLight_yaw = 0.0f;
	mml::Quaternion dirLightRotation = mml::Quaternion({ 0,1,0 }, dirLight_yaw) * mml::Quaternion({ 1,0,0 }, dirLight_pitch);

	Entity* dirLightEntity = commonEntityFactory::create_entity__DirectionalLight(
		dirLightRotation, { 1,1,1 }, { 0,0,0 },
		shadowmapWidth, shadowmapWidth
	);

	// get handle to graphics' renderer commands
	RendererCommands* const rendererCommands = Graphics::get_renderer_commands();
	rendererCommands->setClearColor({ 0.25f,0.25f,0.25f,1 });
	rendererCommands->init(); // this whole RenderCommands::init() - function kind of stupid and shit..

	while (!program.isCloseRequested())
	{

		if (InputHandler::is_key_down(FUNGINE_KEY_ESCAPE))
			program.get_window()->close();

		// To control light direction
		float dirLightRotSpeed = 0.01f;

		if (InputHandler::is_key_down(FUNGINE_KEY_LEFT)) dirLight_yaw -= dirLightRotSpeed;
		if (InputHandler::is_key_down(FUNGINE_KEY_RIGHT)) dirLight_yaw += dirLightRotSpeed;

		if (InputHandler::is_key_down(FUNGINE_KEY_UP)) dirLight_pitch += dirLightRotSpeed;
		if (InputHandler::is_key_down(FUNGINE_KEY_DOWN)) dirLight_pitch -= dirLightRotSpeed;


		mml::Quaternion dirLightRot_pitch({ 1,0,0 }, dirLight_pitch);
		mml::Quaternion dirLightRot_yaw({ 0,1,0 }, dirLight_yaw);

		mml::Quaternion totalDirLightRotation = dirLightRot_yaw * dirLightRot_pitch;
		totalDirLightRotation.normalize();

		std::shared_ptr<Transform> lightTransform = dirLightEntity->getComponent<Transform>();
		lightTransform->setRotation(totalDirLightRotation);

		camController.update();

		program.update();
		Graphics::render();
	}

	return 0;
}
#endif