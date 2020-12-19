
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

#include "utils/modelLoading/ModelLoading.h"

#include "entities/Entity.h"

#include <stdio.h>
#include <cmath>

using namespace fungine;
using namespace core;
using namespace components;
using namespace graphics;
using namespace entities;

int main(int argc, const char** argv)
{
	Program program;
	
	float triVertices[9] =
	{
		0.0f,1.0f,0.0f,
		-1.0f,-1.0f,0.0f,
		1.0f,-1.0f,0.0f
	};
	float quadVertices[12] =
	{
		-1.0f,1.0f,0.0f,
		-1.0f,-1.0f,0.0f,
		1.0f,1.0f,0.0f,
		1.0f,-1.0f,0.0f
	};

	std::vector<float> vertexData_triangle;
	for (int i = 0; i < 9; i++)	vertexData_triangle.push_back(triVertices[i]);
	std::vector<float> vertexData_quad;
	for (int i = 0; i < 12; i++)	vertexData_quad.push_back(quadVertices[i]);

	unsigned int indices_triangle[] =
	{
		0,1,2
	};
	unsigned int indices_quad[] =
	{
		0,1,2,3
	};
	std::vector<unsigned int> indicesData_triangle;
	for (int i = 0; i < 3; i++) indicesData_triangle.push_back(indices_triangle[i]);
	std::vector<unsigned int> indicesData_quad;
	for (int i = 0; i < 4; i++) indicesData_quad.push_back(indices_quad[i]);

	VertexBufferLayout layout(
	{
		{ 0, ShaderDataType::Float3 }
	});
	VertexBuffer* vertexBuffer_triangle = VertexBuffer::create_vertex_buffer(vertexData_triangle, BufferUsage::StaticDraw, layout);
	IndexBuffer* indexBuffer_triangle = IndexBuffer::create_index_buffer(indicesData_triangle);

	
	// mesh for testing framebuffers
	VertexBuffer* vertexBuffer_quad = VertexBuffer::create_vertex_buffer(vertexData_quad, BufferUsage::StaticDraw, layout);
	IndexBuffer* indexBuffer_quad = IndexBuffer::create_index_buffer(indicesData_quad);
	Mesh* mesh_quad = Mesh::create_mesh({ vertexBuffer_quad }, indexBuffer_quad, DrawType::TriangleStrips);

	
	// Framebuffer testing
	Framebuffer* multisampledFramebuffer = Framebuffer::create_framebuffer(800, 600, 4);
	multisampledFramebuffer->addColorAttachment();
	Texture* framebufferTexture = multisampledFramebuffer->getColorAttachment(0);

	// Texture creation
	ImageData* imgDat = ImageData::load_image("res/testTexture.png");
	Texture* texture = Texture::create_texture(imgDat, imgDat->getWidth(), imgDat->getHeight());

	

	// Shader creation
	ShaderStage* vertexShader =		ShaderStage::create_shader_stage("res/shaders/StaticVertexShader.shader", ShaderStageType::VertexShader);
	ShaderStage* pixelShader =		ShaderStage::create_shader_stage("res/shaders/StaticFragmentShader.shader", ShaderStageType::PixelShader);
	
	ShaderProgram* shaderProgram = ShaderProgram::create_shader_program("SimpleStaticShader", vertexShader, pixelShader);

	// Create perspective projection matrix
	const std::unique_ptr<Window>& programWindow = Program::get_window();
	float aspectRatio = (float)(programWindow->getWidth()) / (float)(programWindow->getHeight());
	mml::Matrix4 projectionMatrix(1.0f);
	mml::create_perspective_projection_matrix(projectionMatrix, 70.0f, aspectRatio, 0.1f, 1000.0f);

	mml::Matrix4 viewMatrix(1.0f);

	//mml::Matrix4 transformationMatrix_tri;
	mml::Matrix4 transformationMatrix_quad;
	
	// Material creation
	Material* testFramebufferMaterial = Material::create_material(
		shaderProgram,
		{
			ShaderUniform<mml::Matrix4>("transformationMatrix", &transformationMatrix_quad),
			ShaderUniform<mml::Matrix4>("projectionMatrix", &projectionMatrix),
			ShaderUniform<mml::Matrix4>("viewMatrix", &viewMatrix)
		}
	);
	testFramebufferMaterial->setTexture(0, framebufferTexture);


	// get handle to graphics' renderer commands
	RendererCommands * const rendererCommands = Graphics::get_renderer_commands();
	rendererCommands->setClearColor({ 0,0,1,1 });


	// Testing entity creation
	Entity testEntity;
	// Test entity components
	Transform* component_entityTransform = new Transform({ 0,0,0 }, { {0,1,0}, 0 }, { 1,1,1 });
	Mesh* component_entityMesh = Mesh::create_mesh({ vertexBuffer_triangle }, indexBuffer_triangle, DrawType::TriangleStrips);
	
	Material* component_entityMaterial = Material::create_material(
		shaderProgram,
		{
			ShaderUniform<mml::Matrix4>("transformationMatrix", &component_entityTransform->getTransformationMatrix()),
			ShaderUniform<mml::Matrix4>("projectionMatrix", &projectionMatrix),
			ShaderUniform<mml::Matrix4>("viewMatrix", &viewMatrix)
		},
		{ texture }
	);

	testEntity.addComponent(component_entityTransform);
	testEntity.addComponent(component_entityMesh);
	testEntity.addComponent(component_entityMaterial);


	while (!program.isCloseRequested())
	{
		// Update transformations
		float rotation = glfwGetTime() * 0.9f;
		//mml::create_transformation_matrix(transformationMatrix_tri, { 0,0,-5.0f }, { {0,1,0}, rotation }, { 1,1,1 });
		component_entityTransform->setRotation({ { 1.0f,1.0f,0.0f }, rotation });
		component_entityTransform->setScale({ 1,1,1 });
		component_entityTransform->setPosition({ 0,0,-8 });
		
		mml::create_transformation_matrix(transformationMatrix_quad, { 0.0f,0,-1.0f }, { {0,1,0}, 0 }, { 1,1,1 });

		// Update view
		mml::create_view_matrix(viewMatrix, { 0,0,1 }, { {0,1,0}, 0 });
		
		//rendererCommands->bindFramebuffer(multisampledFramebuffer);
		rendererCommands->clear();


		// render 3d triangle 1
		Mesh* entityMesh = testEntity.getComponent<Mesh>();
		rendererCommands->bindMaterial(testEntity.getComponent<Material>());
		rendererCommands->bindMesh(entityMesh);
		rendererCommands->drawIndices(entityMesh);
		rendererCommands->unbindMesh(entityMesh);

		/*
		rendererCommands->unbindFramebuffer(multisampledFramebuffer);
		rendererCommands->clear();

		rendererCommands->bindMaterial(testFramebufferMaterial);
		// render quad for framebuffer testing
		rendererCommands->bindMesh(mesh_quad);
		rendererCommands->drawIndices(mesh_quad);
		rendererCommands->unbindMesh(mesh_quad);*/

		program.update();
	}

	
	delete mesh_quad;

	return 0;
}


/*


Graphics::submit(mesh1);
Graphics::submit(mesh2);



*/