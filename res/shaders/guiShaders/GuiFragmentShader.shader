#version 330 core

in vec2 var_texCoord;

uniform sampler2D tex;

out vec4 outColor;

void main(void)
{
	outColor = texture(tex, var_texCoord);
}