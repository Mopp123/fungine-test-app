#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 pos;
layout(location = 2) in vec2 scale;
layout(location = 3) in vec2 textureOffset;

uniform mat4 projectionMatrix;
uniform float texRowCount;

out vec2 var_texCoord;

void main(void)
{

	gl_Position = projectionMatrix * (vec4((vertexPos) * vec3(scale.x, scale.y, 1.0) + pos, 1.0));

	var_texCoord = (vec2((vertexPos.x + 1.0) / 2.0, (vertexPos.y + 1.0) / 2.0) + textureOffset) * texRowCount;
}