#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 transformationMatrix;

out vec2 var_uv;

void main(void)
{

	gl_Position = transformationMatrix * vec4(position, 1.0);

	var_uv = position.xy * 0.5 + 0.5;
}