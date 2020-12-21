
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
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
#include "components/rendering/renderers/TerrainRenderer.h"

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

int main(int argc, const char** argv)
{
	Program program("FunGINe engine demo", 1024, 768);

	// Texture creation
	ImageData* imgDat = ImageData::load_image("res/testTexture.png");
	Texture* texture = Texture::create_texture(imgDat, imgDat->getWidth(), imgDat->getHeight());

	// Load all terrain textures
	ImageData* imgDat_blendmap = ImageData::load_image("res/IslandsBlendmap.png");
	
	ImageData* imgDat_dirt_diffuse = ImageData::load_image("res/textures/desert_mud_d.jpg");
	ImageData* imgDat_dirt_normal = ImageData::load_image("res/textures/desert_mud_n.jpg");

	ImageData* imgDat_grass1_diffuse = ImageData::load_image("res/textures/moss_ground_d.jpg");
	ImageData* imgDat_grass2_diffuse = ImageData::load_image("res/textures/moss_plants_d.jpg");
	ImageData* imgDat_grass_specular = ImageData::load_image("res/textures/moss_plants_s.jpg");
	ImageData* imgDat_grass_normal = ImageData::load_image("res/textures/moss_plants_n.jpg");

	ImageData* imgDat_cliff_diffuse = ImageData::load_image("res/textures/jungle_mntn2_d.jpg");
	ImageData* imgDat_cliff_specular = ImageData::load_image("res/textures/jungle_mntn2_h.jpg");
	ImageData* imgDat_cliff_normal = ImageData::load_image("res/textures/jungle_mntn2_n.jpg");

	Texture* texture_blendmap = Texture::create_texture(imgDat_blendmap);
	
	Texture* texture_dirt_diffuse = Texture::create_texture(imgDat_dirt_diffuse);
	Texture* texture_dirt_normal = Texture::create_texture(imgDat_dirt_normal);

	Texture* texture_grass1_diffuse = Texture::create_texture(imgDat_grass1_diffuse);
	Texture* texture_grass2_diffuse = Texture::create_texture(imgDat_grass2_diffuse);

	Texture* texture_grass_specular = Texture::create_texture(imgDat_grass_specular);
	Texture* texture_grass_normal = Texture::create_texture(imgDat_grass_normal);

	Texture* texture_cliff_diffuse = Texture::create_texture(imgDat_cliff_diffuse);
	Texture* texture_cliff_specular = Texture::create_texture(imgDat_cliff_specular);
	Texture* texture_cliff_normal = Texture::create_texture(imgDat_cliff_normal);

	// Create shader for terrain rendering
	ShaderStage* terrainVertexShader = ShaderStage::create_shader_stage("res/shaders/TerrainVertexShader.shader", ShaderStageType::VertexShader);
	ShaderStage* terrainFragmentShader = ShaderStage::create_shader_stage("res/shaders/TerrainFragmentShader.shader", ShaderStageType::PixelShader);
	ShaderProgram* terrainShader = ShaderProgram::create_shader_program("TerrainShader", terrainVertexShader, terrainFragmentShader);

	// Create shader for mesh rendering
	ShaderStage* meshVertexShader = ShaderStage::create_shader_stage("res/shaders/StaticVertexShader.shader", ShaderStageType::VertexShader);
	ShaderStage* meshFragmentShader = ShaderStage::create_shader_stage("res/shaders/StaticFragmentShader.shader", ShaderStageType::PixelShader);
	ShaderProgram* meshShader = ShaderProgram::create_shader_program("MeshShader", meshVertexShader, meshFragmentShader);


	std::shared_ptr<Material> terrainMaterial = Material::create_material__default3D(
		terrainShader,
		{
			{"texture_blendmap", texture_blendmap},

			{"texture_black_diffuse", texture_dirt_diffuse },	{"texture_black_specular", texture_dirt_diffuse },	{"texture_black_normal", texture_dirt_normal },
			{"texture_red_diffuse", texture_grass1_diffuse },	{"texture_red_specular", texture_grass_specular },	{"texture_red_normal", texture_grass_normal },
			{"texture_green_diffuse", texture_grass2_diffuse }, {"texture_green_specular", texture_grass_specular },{"texture_green_normal", texture_grass_normal },
			{"texture_blue_diffuse", texture_cliff_diffuse },	{"texture_blue_specular", texture_cliff_specular },	{"texture_blue_normal", texture_cliff_normal }
		}
	);

	// Load terrain model
	std::vector<std::shared_ptr<Mesh>> terrainMeshes;
	std::vector<Texture*> terrainMesh_textures;
	std::vector<std::shared_ptr<Material>> terrainMesh_materials;

	modelLoading::load_model(
		"res/Islands.fbx",
		terrainMeshes, terrainMesh_textures, terrainMesh_materials,
		modelLoading::ModelLoading_PostProcessFlags::JoinIdenticalVertices |
		modelLoading::ModelLoading_PostProcessFlags::Triangulate |
		modelLoading::ModelLoading_PostProcessFlags::FlipUVs |
		modelLoading::ModelLoading_PostProcessFlags::CalcTangentSpace,
		true
	);
	// Find the actual islands mesh from all the meshes
	std::shared_ptr<Mesh>& islandsMesh = terrainMeshes[0];
	for (std::shared_ptr<Mesh>& m : terrainMeshes)
	{
		printf("mesh name: %s\n", m->getName().c_str());
		if (m->getName() == "Islands")
		{
			islandsMesh = m;
			break;
		}
	}

	// Create renderers
	std::shared_ptr<TerrainRenderer> terrainRenderer = std::make_shared<TerrainRenderer>();
	std::shared_ptr<Renderer> meshRenderer = std::make_shared<Renderer>();

	// Create terrain entity
	Entity* terrainEntity = new Entity;
	float terrainScale = 0.05f;
	std::shared_ptr<Transform> component_terrainTransform = std::make_shared<Transform>(mml::Vector3(0, 0, 0), mml::Quaternion({ 0,1,0 }, 0), mml::Vector3(terrainScale, terrainScale, terrainScale));
	terrainEntity->addComponent(component_terrainTransform);

	terrainEntity->addComponent(terrainMaterial);
	terrainEntity->addComponent(islandsMesh);

	terrainEntity->addComponent(terrainRenderer);

	// Create tree entities
	std::shared_ptr<Material> treeMaterial = terrainMesh_materials[1];
	treeMaterial->setShader(meshShader);

	for (int i = 0; i < terrainMeshes.size(); ++i)
	{
		if (terrainMeshes[i]->getName() == "Islands")
			continue;

		Entity* treeEntity = new Entity;
		float treeScale = terrainScale;
		std::shared_ptr<Transform> component_treeTransform = std::make_shared<Transform>(mml::Vector3(0, 0, 0), mml::Quaternion({ 0,1,0 }, 0), mml::Vector3(treeScale, treeScale, treeScale));
		treeEntity->addComponent(component_treeTransform);

		treeEntity->addComponent(treeMaterial);
		treeEntity->addComponent(terrainMeshes[i]);

		treeEntity->addComponent(meshRenderer);
	}


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


	// get handle to graphics' renderer commands
	RendererCommands* const rendererCommands = Graphics::get_renderer_commands();
	rendererCommands->setClearColor({ 0.25f,0.25f,0.25f,1 });

	while (!program.isCloseRequested())
	{
		rendererCommands->clear();

		// To control light direction
		float dirLightRotSpeed = 0.01f;
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
