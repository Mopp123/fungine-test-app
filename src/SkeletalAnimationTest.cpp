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
	const float camStartPitch = mml::to_radians(0);
	const float camStartYaw = mml::to_radians(0);
	mml::Quaternion camStartRot = mml::Quaternion({ 0,1,0 }, camStartYaw) * mml::Quaternion({ 1,0,0 }, camStartPitch);
	Entity* cameraEntity = commonEntityFactory::create_entity__Camera({ 0,0,2 }, camStartRot);
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

	// Load models
	std::vector<std::shared_ptr<Mesh>> treeMeshes;
	std::vector<Texture*> treeMeshes_textures;
	std::vector<std::shared_ptr<Material>> treeMeshes_materials;

	modelLoading::load_model(
		"res/PalmTree_small.fbx",
		treeMeshes, treeMeshes_textures, treeMeshes_materials,
		modelLoading::ModelLoading_PostProcessFlags::JoinIdenticalVertices |
		modelLoading::ModelLoading_PostProcessFlags::Triangulate |
		modelLoading::ModelLoading_PostProcessFlags::FlipUVs |
		modelLoading::ModelLoading_PostProcessFlags::CalcTangentSpace,
		true,
		instanceCount_trees
	);

	// Create renderers
	std::shared_ptr<NatureRenderer> natureRenderer = std::make_shared<NatureRenderer>();

	// Create shaders for tree rendering
	ShaderStage* treeVertexShader = ShaderStage::create_shader_stage("res/shaders/treeShaders/TreeVertexShader.shader", ShaderStageType::VertexShader);
	ShaderStage* treeFragmentShader = ShaderStage::create_shader_stage("res/shaders/treeShaders/TreeFragmentShader.shader", ShaderStageType::PixelShader);
	ShaderProgram* treeShader = ShaderProgram::create_shader_program("TreeShader", treeVertexShader, treeFragmentShader);
	// Tree shadow shader
	ShaderStage* treeShadowVertexShader = ShaderStage::create_shader_stage("res/shaders/treeShaders/TreeShadowVertexShader.shader", ShaderStageType::VertexShader);
	ShaderStage* treeShadowFragmentShader = ShaderStage::create_shader_stage("res/shaders/treeShaders/TreeShadowFragmentShader.shader", ShaderStageType::PixelShader);
	ShaderProgram* treeShadowShader = ShaderProgram::create_shader_program("TreeShadowShader", treeShadowVertexShader, treeShadowFragmentShader);


	// Create tree entities
	std::shared_ptr<Material> treeMaterial = treeMeshes_materials[0];
	treeMaterial->setShader(treeShader);
	treeMaterial->setShadowShader(treeShadowShader);

	treeMaterial->setShaderUniform_Float({ "m_windMultiplier", ShaderDataType::Float, 0.000002f });

	// Create entities with models
	treeMeshes[0]->enableShadows(true);
	
	Entity* entity = new Entity;
	const float entityScale = 1;
	std::shared_ptr<Transform> transform = std::make_shared<Transform>(mml::Vector3(1, 0, 0), mml::Quaternion({ 0,1,0 }, 0.0f), mml::Vector3(entityScale, entityScale, entityScale));
	entity->addComponent(transform);
	
	entity->addComponent(treeMaterial);
	entity->addComponent(treeMeshes[0]);
	entity->addComponent(natureRenderer);
	
	Entity* childEntity = new Entity;
	const float childEntityScale = 0.5f;
	std::shared_ptr<Transform> childTransform = std::make_shared<Transform>(mml::Vector3(5, 0, 0), mml::Quaternion({ 0,1,0 }, 0.0f), mml::Vector3(childEntityScale, childEntityScale, childEntityScale));
	childTransform->setParent(transform.get());
	childEntity->addComponent(childTransform);

	childEntity->addComponent(treeMaterial);
	childEntity->addComponent(treeMeshes[0]);
	childEntity->addComponent(natureRenderer);

	while (!program.isCloseRequested())
	{
		mml::Quaternion newRot = mml::Quaternion({ 0,1,0 }, mml::to_radians(Time::get_time() * 10.0f)) * mml::Quaternion({ 1,0,0 }, mml::to_radians(Time::get_time() * 10.0f));
		transform->setRotation(newRot);
		childTransform->createTransformationMatrix();

		std::vector<std::shared_ptr<BatchInstanceData>> instancedDatas = entity->getComponents<BatchInstanceData>();
		std::vector<std::shared_ptr<BatchInstanceData>> instancedDatas2 = childEntity->getComponents<BatchInstanceData>();
		if (!instancedDatas.empty())
			instancedDatas[0]->update();
		
		if (!instancedDatas2.empty())
			instancedDatas2[0]->update();

		//printf("r = %f,%f,%f,%f\n", transform->getRotation().x, transform->getRotation().y, transform->getRotation().z, transform->getRotation().w);
		//printf("p = %f,%f,%f\n", childTransform->getPosition().x, childTransform->getPosition().y, childTransform->getPosition().z);
		//printf("s = %f,%f,%f\n", childTransform->getScale().x, childTransform->getScale().y, childTransform->getScale().z);


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