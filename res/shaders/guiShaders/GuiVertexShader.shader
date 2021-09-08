#version 330 core

layout(location = 0) in vec2 vertexPos;
layout(location = 1) in mat4 transformationMatrix;
// location = 2
// location = 3
// location = 4
layout(location = 5) in vec4 color;
layout(location = 6) in vec4 borderColor;
layout(location = 7) in float borderThickness;
layout(location = 8) in vec2 textureOffset;

uniform mat4 projectionMatrix;
uniform float textureRowCount;

out vec2 var_passVertexPos;
out vec2 var_texCoord;
out vec4 var_color;
out vec4 var_borderColor;
out float var_borderThickness;
out vec2 var_scale;

void main(void)
{

	gl_Position = projectionMatrix * transformationMatrix * vec4(vertexPos, 0.0, 1.0);
	var_texCoord = (vec2(vertexPos.x, vertexPos.y) + textureOffset) / textureRowCount;

	var_color = color;
	var_borderColor = borderColor;
	var_borderThickness = borderThickness;

	// extract scale from the transformation matrix
	mat4 tMat = transformationMatrix;
	// first zero the translation
	tMat[0][3] = 0.0;
	tMat[1][3] = 0.0;
	tMat[2][3] = 0.0;

	float sx = length(vec3(tMat[0][0], tMat[1][0], tMat[2][0]));
	float sy = length(vec3(tMat[0][1], tMat[1][1], tMat[2][1]));

	var_scale = vec2(sx, sy);
	var_passVertexPos = vertexPos * var_scale;
}