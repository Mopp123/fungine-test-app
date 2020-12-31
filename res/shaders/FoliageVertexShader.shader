#version 330 core

layout(location = 0) in vec3 position_modelSpace;
layout(location = 1) in vec2 uvCoord;
layout(location = 2) in vec3 normal;

// below all per instance buffers
layout(location = 4) in mat4 transformationMatrix;
// m2: location = 5
// m3: location = 6
// m4: location = 7

layout(location = 8) in float windInitVal;
// windInitVal: Is used to make the wind anim start at a little different time so all trees wont wiggle at the exactly same phase

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

uniform vec3 directionalLight_direction;
uniform vec3 cameraPos;

uniform float m_windMultiplier;
uniform float time;

uniform mat4 shadowProjMat;
uniform mat4 shadowViewMat;

out vec2 var_uv;
out float var_diffuseFactor;
out vec4 var_shadowCoord;


void main(void)
{
	float windAnim = abs(time - windInitVal);
	float weight = (position_modelSpace.y * position_modelSpace.y) * m_windMultiplier;
	float windEffect_x = sin(windAnim) * weight;
	float windEffect_z = cos(windAnim) * weight;
	vec3 finalVertexPos = position_modelSpace;
	
	finalVertexPos.x += windEffect_x;
	finalVertexPos.z += windEffect_z;

	vec4 vertex_worldSpace = transformationMatrix * vec4(finalVertexPos, 1.0);
	gl_Position = projectionMatrix * viewMatrix * vertex_worldSpace;
	
	var_uv = uvCoord;
	var_diffuseFactor = max(dot(-directionalLight_direction, vec3(0, 1, 0)), 0.0);
	var_shadowCoord = 0.5 + 0.5 * shadowProjMat * shadowViewMat * vertex_worldSpace;
}