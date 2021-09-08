#version 330 core

layout(location = 0) in vec2 vertexPos;
layout(location = 1) in vec4 transform;
layout(location = 2) in vec2 textureOffset;

uniform mat4 projectionMatrix;
uniform float textureRowCount;

out vec2 var_texCoord;

void main(void)
{

	vec2 finalVertexPos = vertexPos * transform.zw + transform.xy;

	gl_Position = projectionMatrix * vec4(finalVertexPos, 0.0, 1.0);

	var_texCoord = (vec2(vertexPos.x, vertexPos.y) + textureOffset) / textureRowCount;
}