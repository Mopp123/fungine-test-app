#include "Fungine.hpp"
#include "controllers/CameraController.h"
#include <stdio.h>
#include <cmath>

using namespace fungine;
using namespace core;
using namespace components;
using namespace rendering;
using namespace entities;
using namespace graphics;
using namespace entities;

// Used for planting vegetation..
static bool get_valid_pixel(ImageData* blendmap, unsigned int px, unsigned int py)
{
	byte pixelRed = blendmap->getColorChannel(px, py, 0);
	byte pixelGreen = blendmap->getColorChannel(px, py, 1);
	if (pixelGreen > 0 || pixelRed > 32)
		return true;
	else
		return false;
}

// randomizes a position from terrain blendmap for vegetation..
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

// just quick way to read config file..
void read_config(
	const std::string& filename,
	unsigned int& out_shadowmapWidth,
	unsigned int& out_windowWidth,
	unsigned int& out_windowHeight,
	unsigned int& out_windowFullscreen,
	unsigned int& out_vSync,

	unsigned int& out_treeCount,
	unsigned int& out_grassCount
)
{
	FILE* file = nullptr;
        // quick fix to get running on linux
        #ifdef _WIN32
	fopen_s(&file, filename.c_str(), "rt");
        #elif __linux__
	file = fopen(filename.c_str(), "rt");
        #endif

	if (!file)
	{
		Debug::log("Location: read_config(\n"
			"const std::string & filename,\n"
			"unsigned int& out_shadowmapWidth,\n"
			"unsigned int& out_windowWidth,\n"
			"unsigned int& out_windowHeight,\n"
			"unsigned int& out_windowFullscreen\n"
			")\n"
			"Failed to read config file from: " + filename,
			DEBUG__ERROR_LEVEL__ERROR
		);
		return;
	}

        // quick fix to get running on linux
        #ifdef _WIN32
	fscanf_s(file, "shadowmapWidth=%d\n", &out_shadowmapWidth);
	fscanf_s(file, "windowWidth=%d\n", &out_windowWidth);
	fscanf_s(file, "windowHeight=%d\n", &out_windowHeight);
	fscanf_s(file, "windowFullscreen=%d\n", &out_windowFullscreen);
	fscanf_s(file, "vSync=%d\n", &out_vSync);
	fscanf_s(file, "treeCount=%d\n", &out_treeCount);
	fscanf_s(file, "grassCount=%d", &out_grassCount);
        #elif __linux__
	fscanf(file, "shadowmapWidth=%d\n", &out_shadowmapWidth);
	fscanf(file, "windowWidth=%d\n", &out_windowWidth);
	fscanf(file, "windowHeight=%d\n", &out_windowHeight);
	fscanf(file, "windowFullscreen=%d\n", &out_windowFullscreen);
	fscanf(file, "vSync=%d\n", &out_vSync);
	fscanf(file, "treeCount=%d\n", &out_treeCount);
	fscanf(file, "grassCount=%d", &out_grassCount);
        #endif

	fclose(file);
}

int main(int argc, const char** argv)
{
	unsigned int windowWidth = 1024;
	unsigned int windowHeight = 768;
	unsigned int windowFullscreen = 0;
	unsigned int shadowmapWidth = 1024;
	unsigned int vSync = 0;

	unsigned int instanceCount_trees = 1000;
	unsigned int instanceCount_grass = 100000;

	read_config("res/config.txt", shadowmapWidth, windowWidth, windowHeight, windowFullscreen, vSync, instanceCount_trees, instanceCount_grass);

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

	// Create shaders for tree rendering
	ShaderStage* treeVertexShader = ShaderStage::create_shader_stage("res/shaders/treeShaders/TreeVertexShader.shader", ShaderStageType::VertexShader);
	ShaderStage* treeFragmentShader = ShaderStage::create_shader_stage("res/shaders/treeShaders/TreeFragmentShader.shader", ShaderStageType::PixelShader);
	ShaderProgram* treeShader = ShaderProgram::create_shader_program("TreeShader", treeVertexShader, treeFragmentShader);
	// Tree shadow shader
	ShaderStage* treeShadowVertexShader = ShaderStage::create_shader_stage("res/shaders/treeShaders/TreeShadowVertexShader.shader", ShaderStageType::VertexShader);
	ShaderStage* treeShadowFragmentShader = ShaderStage::create_shader_stage("res/shaders/treeShaders/TreeShadowFragmentShader.shader", ShaderStageType::PixelShader);
	ShaderProgram* treeShadowShader = ShaderProgram::create_shader_program("TreeShadowShader", treeShadowVertexShader, treeShadowFragmentShader);

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

	modelLoading::load_model(
		"res/PalmTree_straight.fbx",
		treeMeshes, treeMeshes_textures, treeMeshes_materials,
		modelLoading::ModelLoading_PostProcessFlags::JoinIdenticalVertices |
		modelLoading::ModelLoading_PostProcessFlags::Triangulate |
		modelLoading::ModelLoading_PostProcessFlags::FlipUVs |
		modelLoading::ModelLoading_PostProcessFlags::CalcTangentSpace,
		true,
		instanceCount_trees
	);

	// Load grass
	std::vector<std::shared_ptr<Mesh>> grassMeshes;
	std::vector<Texture*> grassMeshes_textures;
	std::vector<std::shared_ptr<Material>> grassMeshes_materials;

	modelLoading::load_model(
		"res/Grass.fbx",
		grassMeshes, grassMeshes_textures, grassMeshes_materials,
		modelLoading::ModelLoading_PostProcessFlags::JoinIdenticalVertices |
		modelLoading::ModelLoading_PostProcessFlags::Triangulate |
		modelLoading::ModelLoading_PostProcessFlags::FlipUVs,
		true,
		instanceCount_grass
	);


	// Create renderers
	std::shared_ptr<TerrainRenderer> terrainRenderer = std::make_shared<TerrainRenderer>();
	std::shared_ptr<NatureRenderer> natureRenderer = std::make_shared<NatureRenderer>();

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
	treeMaterial->setShadowShader(treeShadowShader);

	treeMaterial->setShaderUniform_Float({ "m_windMultiplier", ShaderDataType::Float, 0.000002f });

	treeMeshes[0]->enableShadows(true);
	std::vector<Entity*> testTrees;
	for (int i = 0; i < instanceCount_trees; ++i)
	{
		Entity* treeEntity = new Entity(true);
		testTrees.push_back(treeEntity);


		float treeScale = 0.001f;

		float randomYaw = (float)(std::rand() % 100) * 0.01f * 2 - 1.0f;
		std::shared_ptr<Transform> component_transform = std::make_shared<Transform>(mml::Vector3(0,0,0), mml::Quaternion({ 0,1,0 }, randomYaw), mml::Vector3(treeScale, treeScale, treeScale));
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
	grassMaterial->setShaderUniform_Float({ "m_windMultiplier", ShaderDataType::Float, 0.001f });
	grassMeshes[0]->enableShadows(false); // Don't render foliage to shadowmap

	for (int i = 0; i < instanceCount_grass; ++i)
	{
		Entity* grassEntity = new Entity(true);
		float grassScale = 0.025f;
		float randomYaw = (float)(std::rand() % 100) * 0.01f * 2 - 1.0f;
		std::shared_ptr<Transform> component_transform = std::make_shared<Transform>(mml::Vector3(0, 0, 0), mml::Quaternion({ 0,1,0 }, randomYaw), mml::Vector3(grassScale, grassScale, grassScale));
		grassEntity->addComponent(component_transform);

		grassEntity->addComponent(grassMaterial);

		grassEntity->addComponent(grassMeshes[0]);
		grassEntity->addComponent(natureRenderer);

		vegetationEntities.push_back(grassEntity);
	}

	plant_vegetation(terrain, imgDat_blendmap, vegetationEntities);

	// get handle to graphics' renderer commands
	RendererCommands* const rendererCommands = Graphics::get_renderer_commands();
	rendererCommands->setClearColor({ 0.25f,0.25f,0.25f,1 });
	rendererCommands->init();

	// Create a quad on screen to debug shadowmap
	float shadowmapQuadSize = 0.4f;
	Entity* shadowmapDebugEntity = commonEntityFactory::create_entity__Plane({ -1.0f + shadowmapQuadSize, 0.5f,0 }, { {0,1,0}, 0 }, { shadowmapQuadSize,shadowmapQuadSize,shadowmapQuadSize });
	std::shared_ptr<Transform> shadowmapDebugTransform = shadowmapDebugEntity->getComponent<Transform>();
	std::shared_ptr<Mesh> shadowmapDebugMesh = shadowmapDebugEntity->getComponent<Mesh>();

	// create shader to draw shadow map debugging..
	ShaderStage* guiVertexShader = ShaderStage::create_shader_stage("res/shaders/guiShaders/GuiVertexShader_TEST.shader", ShaderStageType::VertexShader);
	ShaderStage* guiFragmentShader = ShaderStage::create_shader_stage("res/shaders/guiShaders/GuiFragmentShader_TEST.shader", ShaderStageType::PixelShader);
	ShaderProgram* guiShader = ShaderProgram::create_shader_program("GuiShader", guiVertexShader, guiFragmentShader);

	const Texture* shadowmapTexture = dirLightEntity->getComponent<DirectionalLight>()->getShadowCaster().getShadowmapTexture();

	// Test GUI component rendering
	//std::shared_ptr<GUIRenderer> guiRenderer = std::make_shared<GUIRenderer>();
	std::shared_ptr<GUITextRenderer> textRenderer = std::make_shared<GUITextRenderer>();

	//std::vector<Entity*> guiPanelEntity = guiEntityFactory::create_panel_entity(512, 256, 256, 512, "Testing panel",0,32);
	//guiPanelEntity[1]->addComponent(guiRenderer);
	//guiPanelEntity[2]->addComponent(textRenderer);

	// Test text rendering

	//Entity* textEntity = new Entity;
	//std::shared_ptr<Transform> textTransform = std::make_shared<Transform>(
	//	mml::Vector3(0, 512, 0), mml::Quaternion({ 0,1,0 }, 0), mml::Vector3(1, 1, 1));
	//std::shared_ptr<GUIText> guiText = std::make_shared<GUIText>(
	//	"Testing text rendering asd123... a very lengthy text string here."
	//);
	//textEntity->addComponent(textTransform);
	//textEntity->addComponent(textRenderer);
	//textEntity->addComponent(guiText);

	// Make different looking font for FPS text
	Font* font_fpsText = new Font("res/default/fonts/TestFont.ttf", 10, { 1,1,0,1 });

	Entity* FPSTextEntity = new Entity;
	std::shared_ptr<Transform> FPSTextTransform = std::make_shared<Transform>(
		mml::Vector3(0, 0, 0), mml::Quaternion({ 0,1,0 }, 0), mml::Vector3(1.0f, 1.0f, 1.0f));
	std::shared_ptr<GUIText> FPSText = std::make_shared<GUIText>("FPS: ", font_fpsText);
	FPSTextEntity->addComponent(FPSTextTransform);
	FPSTextEntity->addComponent(textRenderer);
	FPSTextEntity->addComponent(FPSText);

	while (!program.isCloseRequested())
	{
		/*
		std::vector<std::shared_ptr<BatchInstanceData>> panelInstancedDatas1 = guiPanelEntity[1]->getComponents<BatchInstanceData>();
		std::vector<std::shared_ptr<BatchInstanceData>> panelInstancedDatas2 = guiPanelEntity[2]->getComponents<BatchInstanceData>();

		if (!panelInstancedDatas1.empty())
			panelInstancedDatas1[0]->update();

		if (!panelInstancedDatas2.empty())
			panelInstancedDatas2[0]->update();
		*/

		FPSText->setText("FPS: " + std::to_string(Time::get_fps()));

		// Test batched entity dynamic deleting
		if (InputHandler::is_key_down(FUNGINE_KEY_E) && !vegetationEntities.empty())
		{
			int randIndex = std::rand() % vegetationEntities.size();
			if (randIndex >= 0 && randIndex < vegetationEntities.size())
			{
				Entity* e = vegetationEntities[randIndex];
				if (e)
				{
					std::shared_ptr<BatchInstanceData> b = e->getComponent<BatchInstanceData>();
					if (b)
					{
						natureRenderer->removeFromRenderList(e);
						vegetationEntities.erase(vegetationEntities.begin() + randIndex);
						delete e;
					}
				}
			}
		}

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

		// glue cam to ground for testing
		/*std::shared_ptr<Transform> camTransform = cameraEntity->getComponent<Transform>();
		camTransform->update();
		mml::Vector3 camPos = camTransform->getPosition();
		camPos.y = terrain->getHeightAt(camPos.x, camPos.z) + 2.0f;

		camTransform->setPosition(camPos);
		*/

		program.update();
		Graphics::render();

		// render shadowmap into quad on screen for debugginh purposes..
                /*
		rendererCommands->bindShader(guiShader);
		guiShader->setUniform("transformationMatrix", shadowmapDebugTransform->getTransformationMatrix());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, shadowmapTexture->getID());
		rendererCommands->bindMesh(shadowmapDebugMesh.get());
		rendererCommands->drawIndices(shadowmapDebugMesh.get());
		glBindTexture(GL_TEXTURE_2D, 0);
		rendererCommands->unbindMesh(shadowmapDebugMesh.get());
		rendererCommands->unbindShader();
                */
	}

	return 0;
}
