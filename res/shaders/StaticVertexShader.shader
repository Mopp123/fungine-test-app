#version 330 core

layout(location = 0) in vec3 position_modelSpace;
layout(location = 1) in vec2 uvCoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 transformationMatrix;

uniform vec3 directionalLight_direction;
//uniform float time;

uniform vec3 cameraPos;

out vec2 var_uv;
out vec3 var_fragPos;
out vec3 var_toCameraVec;

out vec3 var_dirLightDirection;
out mat3 var_tbnMatrix;

void main(void)
{
	vec4 vertex_worldSpace = transformationMatrix * vec4(position_modelSpace, 1.0);
	gl_Position = projectionMatrix * viewMatrix * vertex_worldSpace;
	
	var_uv = uvCoord;
	
	vec3 t = normalize((transformationMatrix * vec4(tangent, 0.0)).xyz);
	vec3 n = normalize((transformationMatrix * vec4(normal, 0.0)).xyz);
	vec3 b = normalize(cross(n, t));

	// We calc all lighting in tangent space so we need to invert our tbn matrix.
	var_tbnMatrix = transpose(mat3(t, b, n)); // *transposing orthogonal matrix is the same as inverting it

	// Transform all lighting calc stuff into tangent space
	var_fragPos =			var_tbnMatrix * vertex_worldSpace.xyz;
	var_toCameraVec =		var_tbnMatrix * normalize(cameraPos - vertex_worldSpace.xyz);
	var_dirLightDirection = var_tbnMatrix * normalize(directionalLight_direction);
}