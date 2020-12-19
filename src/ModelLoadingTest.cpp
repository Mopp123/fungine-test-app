
#include <glew.h>
#include <glfw3.h>
#include "core/Program.h"
#include "utils/myMathLib/MyMathLib.h"
#include "core/Common.h"
#include "core/Debug.h"

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

#include "utils/modelLoading/ModelLoading.h"

#include "entities/Entity.h"
#include "entities/commonEntitiesLib/CommonEntityFactory.h"

#include "controllers/CameraController.h"

#include <stdio.h>
#include <cmath>

/*
	"Comment tags"( all of them starts with '*->' ):

	*->TEMP = very temporary testing thing that should be quickly removed/changed to more final form.
*/

using namespace fungine;
using namespace core;
using namespace components;
using namespace rendering;
using namespace entities;
using namespace graphics;
using namespace entities;
using namespace modelLoading;

int main(int argc, const char** argv)
{
	Program program;

	// Texture creation
	ImageData* imgDat = ImageData::load_image("res/testTexture.png");
	Texture* texture = Texture::create_texture(imgDat, imgDat->getWidth(), imgDat->getHeight());

	// Shader creation
	ShaderStage* vertexShader = ShaderStage::create_shader_stage("res/shaders/StaticVertexShader.shader", ShaderStageType::VertexShader);
	ShaderStage* pixelShader = ShaderStage::create_shader_stage("res/shaders/StaticFragmentShader.shader", ShaderStageType::PixelShader);

	ShaderProgram* shaderProgram = ShaderProgram::create_shader_program("SimpleStaticShader", vertexShader, pixelShader);

	// Create perspective projection matrix
	float aspectRatio = (float)(Window::get_width()) / (float)(Window::get_height());
	mml::Matrix4 initialProjMat(1.0f);
	mml::create_perspective_projection_matrix(initialProjMat, 70.0f, aspectRatio, 0.1f, 1000.0f);

	// Create camera entity
	Entity* cameraEntity = commonEntityFactory::create_entity__Camera({ 0,3,8 }, { {0,1,0}, 0 }, initialProjMat);
	CameraController camController(cameraEntity);

	// Create directional light entity
	mml::Vector3 dirLightDirection(0, -1, -1);
	dirLightDirection.normalize();
	Entity* dirLightEntity = commonEntityFactory::create_entity__DirectionalLight(dirLightDirection, { 1,1,1 }, { 0,0,0 });
	float dirLight_pitch = 1.57079633f * 0.5f;
	float dirLight_yaw = 0.0f;

	// Testing model loading
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<Texture*> mesh_textures;
	std::vector<std::shared_ptr<Material>> mesh_materials;

	load_model(
		"res/Islands.fbx", 
		meshes, mesh_textures, mesh_materials,
		ModelLoading_PostProcessFlags::JoinIdenticalVertices | 
		ModelLoading_PostProcessFlags::Triangulate | 
		ModelLoading_PostProcessFlags::FlipUVs |
		ModelLoading_PostProcessFlags::CalcTangentSpace,
		true
	);

	std::vector<Entity*> allEntities;
	for (int i = 0; i < meshes.size(); ++i)
	{
		// Testing entity creation
		Entity* entity = new Entity;
		// Test entity components
		std::shared_ptr<Transform> component_entityTransform = std::make_shared<Transform>(mml::Vector3(0, 0, 0), mml::Quaternion({ 0,1,0 }, 0), mml::Vector3(1, 1, 1));
		component_entityTransform->setPosition({ 0,0,-30 });
		component_entityTransform->setScale({ 30,30,30 });
		component_entityTransform->setRotation({ { 1,0,0 }, -1.57079633f });
		entity->addComponent(component_entityTransform);

		std::shared_ptr<Material> material = mesh_materials[i];
		material->setShader(shaderProgram);
		entity->addComponent(material);
		entity->addComponent(meshes[i]);

		entity->addComponent(std::make_shared<Renderer>());
	}

	// get handle to graphics' renderer commands
	RendererCommands* const rendererCommands = Graphics::get_renderer_commands();
	rendererCommands->setClearColor({ 0.25f,0.25f,0.25f,1 });

	while (!program.isCloseRequested())
	{
		rendererCommands->clear();

		// To control light direction
		float dirLightRotSpeed = 0.0008f;
		float y = 0.0f;
		float p = 0.0f;

		if (InputHandler::is_key_down(FUNGINE_KEY_LEFT)) y -= dirLightRotSpeed;
		if (InputHandler::is_key_down(FUNGINE_KEY_RIGHT)) y += dirLightRotSpeed;

		if (InputHandler::is_key_down(FUNGINE_KEY_UP)) p += dirLightRotSpeed;
		if (InputHandler::is_key_down(FUNGINE_KEY_DOWN)) p -= dirLightRotSpeed;

		DirectionalLight* dirLightComponent = dirLightEntity->getComponent<DirectionalLight>().get();
		mml::Vector3 currentDirection = dirLightComponent->getDirection();
		currentDirection.normalize();

		mml::Quaternion dirLightRot_pitch({ 1,0,0 }, p);
		mml::Quaternion dirLightRot_yaw({ 0,1,0 }, y);
		dirLightRot_pitch.normalize();
		dirLightRot_yaw.normalize();

		mml::Quaternion totalDirLightRotation = dirLightRot_yaw * dirLightRot_pitch;
		totalDirLightRotation.normalize();

		mml::Quaternion d(currentDirection.x, currentDirection.y, currentDirection.z, 0);

		mml::Quaternion finalDirLightDirection = totalDirLightRotation * d * totalDirLightRotation.conjugate();
		d.normalize();

		dirLightComponent->setDirection({ finalDirLightDirection.x, finalDirLightDirection.y ,finalDirLightDirection.z });

		camController.update();

		program.update();
	}

	return 0;
}
