
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
	"Comment tags"( all of them starts with '*->' ):

	*->TEMP = very temporary testing thing that should be quickly removed/changed to more final form.
	*->TODO = ..to doo
*/

using namespace fungine;
using namespace core;
using namespace components;
using namespace rendering;
using namespace entities;
using namespace graphics;
using namespace entities;


static bool get_valid_pixel(ImageData* blendmap, unsigned int px, unsigned int py)
{
	byte pixelRed = blendmap->getColorChannel(px, py, 0);
	byte pixelGreen = blendmap->getColorChannel(px, py, 1);
	if (pixelGreen > 0 || pixelRed > 32)
		return true;
	else
		return false;
}

// randomizes a position from a map
static void plant_vegetation(Terrain* terrain, ImageData* blendmap, std::vector<Entity*>& entities)
{
	float terrainTileSize = terrain->getTileWidth();
	
	float scalingFactor = (terrain->getTileWidth() * terrain->getVerticesPerRow()) / blendmap->getWidth();
	for (Entity* e : entities)
	{
		unsigned int px = 0;
		unsigned int py = 0;

		bool foundPixel = false;
		while (!foundPixel)
		{
			px = std::rand() % blendmap->getWidth();
			py = std::rand() % blendmap->getHeight();
			foundPixel = get_valid_pixel(blendmap, px, py);
		}

		std::shared_ptr<Transform> t = e->getComponent<Transform>();
		float xPos = (float)px * scalingFactor;
		float zPos = (float)py * scalingFactor;
		float yPos = terrain->getHeightAt(xPos, zPos);
		t->setPosition({ xPos, yPos, zPos });
	}
}

int main(int argc, const char** argv)
{
	Program program("FunGINe engine demo", 1024, 768, false);

	// Create perspective projection matrix
	float aspectRatio = (float)(Window::get_width()) / (float)(Window::get_height());
	mml::Matrix4 initialProjMat(1.0f);
	mml::create_perspective_projection_matrix(initialProjMat, 70.0f, aspectRatio, 0.1f, 500.0f);

	// Create camera entity
	Entity* cameraEntity = commonEntityFactory::create_entity__Camera({ 0,3,8 }, { {0,1,0}, 0 }, initialProjMat);
	CameraController camController(cameraEntity);


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

	// Create shader for tree rendering
	ShaderStage* treeVertexShader = ShaderStage::create_shader_stage("res/shaders/TreeVertexShader.shader", ShaderStageType::VertexShader);
	ShaderStage* treeFragmentShader = ShaderStage::create_shader_stage("res/shaders/TreeFragmentShader.shader", ShaderStageType::PixelShader);
	ShaderProgram* treeShader = ShaderProgram::create_shader_program("TreeShader", treeVertexShader, treeFragmentShader);

	// Create shader for foliage/grass rendering
	ShaderStage* foliageVertexShader = ShaderStage::create_shader_stage("res/shaders/FoliageVertexShader.shader", ShaderStageType::VertexShader);
	ShaderStage* foliageFragmentShader = ShaderStage::create_shader_stage("res/shaders/FoliageFragmentShader.shader", ShaderStageType::PixelShader);
	ShaderProgram* foliageShader = ShaderProgram::create_shader_program("FoliageShader", foliageVertexShader, foliageFragmentShader);

	// Create shader for mesh rendering
	ShaderStage* meshVertexShader = ShaderStage::create_shader_stage("res/shaders/StaticVertexShader.shader", ShaderStageType::VertexShader);
	ShaderStage* meshFragmentShader = ShaderStage::create_shader_stage("res/shaders/StaticFragmentShader.shader", ShaderStageType::PixelShader);
	ShaderProgram* meshShader = ShaderProgram::create_shader_program("MeshShader", meshVertexShader, meshFragmentShader);

	// Load palm trees
	std::vector<std::shared_ptr<Mesh>> treeMeshes;
	std::vector<Texture*> treeMeshes_textures;
	std::vector<std::shared_ptr<Material>> treeMeshes_materials;

	const unsigned int treeInstanceCount = 500;
	modelLoading::load_model(
		"res/PalmTree_straight.fbx",
		treeMeshes, treeMeshes_textures, treeMeshes_materials,
		modelLoading::ModelLoading_PostProcessFlags::JoinIdenticalVertices |
		modelLoading::ModelLoading_PostProcessFlags::Triangulate |
		modelLoading::ModelLoading_PostProcessFlags::FlipUVs |
		modelLoading::ModelLoading_PostProcessFlags::CalcTangentSpace,
		true,
		treeInstanceCount
	);

	// Load grass
	std::vector<std::shared_ptr<Mesh>> grassMeshes;
	std::vector<Texture*> grassMeshes_textures;
	std::vector<std::shared_ptr<Material>> grassMeshes_materials;

	const unsigned int grassInstanceCount = 1000;
	modelLoading::load_model(
		"res/Grass.fbx",
		grassMeshes, grassMeshes_textures, grassMeshes_materials,
		modelLoading::ModelLoading_PostProcessFlags::JoinIdenticalVertices |
		modelLoading::ModelLoading_PostProcessFlags::Triangulate |
		modelLoading::ModelLoading_PostProcessFlags::FlipUVs,
		true,
		grassInstanceCount
	);


	// Create renderers
	std::shared_ptr<TerrainRenderer> terrainRenderer = std::make_shared<TerrainRenderer>();
	std::shared_ptr<NatureRenderer> natureRenderer = std::make_shared<NatureRenderer>();
	std::shared_ptr<Renderer> meshRenderer = std::make_shared<Renderer>();

	// Generate terrain entity
	ImageData* heightmapImage = ImageData::load_image("res/heightmapTest.png");
	Terrain* terrain = new Terrain(
		5.0f, heightmapImage,
		texture_blendmap,
		texture_dirt_diffuse, texture_dirt_diffuse, texture_dirt_normal,
		texture_grass1_diffuse, texture_grass_specular, texture_grass_normal,
		texture_grass2_diffuse, texture_grass_specular, texture_grass_normal,
		texture_cliff_diffuse, texture_cliff_specular, texture_cliff_normal,
		terrainRenderer,
		terrainShader
	);

	// Create all vegetation entities
	std::vector<Entity*> vegetationEntities;

	// Create tree entities
	std::shared_ptr<Material> treeMaterial = treeMeshes_materials[0];
	treeMaterial->setShader(treeShader);
	float treeWindMultiplier = 0.0000025f;
	ShaderUniform<float> treeShader_windUniform("windMultiplier", &treeWindMultiplier);
	treeMaterial->addUniform(treeShader_windUniform);

	for (int i = 0; i < treeInstanceCount; ++i)
	{
		Entity* treeEntity = new Entity;
		float treeScale = 0.001f;

		float randomYaw = (float)(std::rand() % 100) * 0.01f * 2 - 1.0f;
		std::shared_ptr<Transform> component_transform = std::make_shared<Transform>(mml::Vector3(0,0,0), mml::Quaternion({ 0,1,0 }, randomYaw), mml::Vector3(treeScale, treeScale, treeScale), true);
		treeEntity->addComponent(component_transform);

		treeEntity->addComponent(treeMaterial);
		
		treeEntity->addComponent(treeMeshes[0]);
		treeEntity->addComponent(natureRenderer);

		vegetationEntities.push_back(treeEntity);
	}

	// Create grass entities
	std::shared_ptr<Material> grassMaterial = grassMeshes_materials[0];
	grassMaterial->setTwoSided(true);
	grassMaterial->setShader(foliageShader);
	float foliageWindMultiplier = 0.001f;
	ShaderUniform<float> foliageShader_windUniform("windMultiplier", &foliageWindMultiplier);
	grassMaterial->addUniform(foliageShader_windUniform);

	for (int i = 0; i < grassInstanceCount; ++i)
	{
		Entity* grassEntity = new Entity;
		float grassScale = 0.025f;
		float randomYaw = (float)(std::rand() % 100) * 0.01f * 2 - 1.0f;
		std::shared_ptr<Transform> component_transform = std::make_shared<Transform>(mml::Vector3(0, 0, 0), mml::Quaternion({ 0,1,0 }, randomYaw), mml::Vector3(grassScale, grassScale, grassScale), true);
		grassEntity->addComponent(component_transform);

		grassEntity->addComponent(grassMaterial);

		grassEntity->addComponent(grassMeshes[0]);
		grassEntity->addComponent(natureRenderer);

		vegetationEntities.push_back(grassEntity);
	}

	plant_vegetation(terrain, imgDat_blendmap, vegetationEntities);
	

	// Shader creation
	ShaderStage* vertexShader = ShaderStage::create_shader_stage("res/shaders/StaticVertexShader.shader", ShaderStageType::VertexShader);
	ShaderStage* pixelShader = ShaderStage::create_shader_stage("res/shaders/StaticFragmentShader.shader", ShaderStageType::PixelShader);

	ShaderProgram* shaderProgram = ShaderProgram::create_shader_program("SimpleStaticShader", vertexShader, pixelShader);

	// Create directional light entity
	mml::Vector3 dirLightDirection(0, -1, -1);
	dirLightDirection.normalize();
	unsigned int shadowmapWidth = 1024;
	unsigned int shadowmapHeight = 768;
	Entity* dirLightEntity = commonEntityFactory::create_entity__DirectionalLight(
		dirLightDirection, { 1,1,1 }, { 0,0,0 }, 
		shadowmapWidth, shadowmapHeight
	);
	float dirLight_pitch = 1.57079633f * 0.5f;
	float dirLight_yaw = 0.0f;


	// get handle to graphics' renderer commands
	RendererCommands* const rendererCommands = Graphics::get_renderer_commands();
	rendererCommands->setClearColor({ 0.25f,0.25f,0.25f,1 });
	rendererCommands->init();

	// Create a quad on screen to debug shadowmap
	float shadowmapQuadSize = 0.4f;
	Entity* shadowmapDebugEntity = commonEntityFactory::create_entity__Plane({ -1.0f + shadowmapQuadSize,1.0f - shadowmapQuadSize,0 }, { {0,1,0}, 0 }, { shadowmapQuadSize,shadowmapQuadSize,shadowmapQuadSize });
	std::shared_ptr<Transform> shadowmapDebugTransform = shadowmapDebugEntity->getComponent<Transform>();
	std::shared_ptr<Mesh> shadowmapDebugMesh = shadowmapDebugEntity->getComponent<Mesh>();
	
	// create shader to draw shadow map debugging..
	ShaderStage* guiVertexShader = ShaderStage::create_shader_stage("res/shaders/guiShaders/GuiVertexShader.shader", ShaderStageType::VertexShader);
	ShaderStage* guiFragmentShader = ShaderStage::create_shader_stage("res/shaders/guiShaders/GuiFragmentShader.shader", ShaderStageType::PixelShader);
	ShaderProgram* guiShader = ShaderProgram::create_shader_program("GuiShader", guiVertexShader, guiFragmentShader);

	Texture* shadowmapTexture = dirLightEntity->getComponent<DirectionalLight>()->getShadowCaster().getShadowmapTexture();

	while (!program.isCloseRequested())
	{
		
		if (InputHandler::is_key_down(FUNGINE_KEY_ESCAPE))
			program.get_window()->close();
		
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

		// glue cam to ground for testing
		/*std::shared_ptr<Transform> camTransform = cameraEntity->getComponent<Transform>();
		camTransform->update();
		mml::Vector3 camPos = camTransform->getPosition();
		camPos.y = terrain->getHeightAt(camPos.x, camPos.z) + 2.0f;
		
		camTransform->setPosition(camPos);
		*/

		program.update();
		Graphics::render();

		// render shadow map into quad on screen for debugginh purposes..
		guiShader->bind();
		guiShader->setUniform("transformationMatrix", shadowmapDebugTransform->getTransformationMatrix());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, shadowmapTexture->getID());
		rendererCommands->bindMesh(shadowmapDebugMesh.get());
		rendererCommands->drawIndices(shadowmapDebugMesh.get());
		glBindTexture(GL_TEXTURE_2D, 0);
		rendererCommands->unbindMesh(shadowmapDebugMesh.get());
		guiShader->unbind();
	}

	return 0;
}
