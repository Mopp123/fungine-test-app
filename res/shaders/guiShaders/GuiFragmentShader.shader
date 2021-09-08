#version 330 core

in vec2 var_passVertexPos;
in vec2 var_texCoord;
in vec4 var_color;
in vec4 var_borderColor;
in float var_borderThickness;
in vec2 var_scale;

uniform sampler2D tex;

out vec4 outColor;

void main(void)
{
	
	float borderThresholdX = var_scale.x - var_borderThickness * 2.0; // multiply by 2 because we have borders on left AND right
	float borderThresholdY = var_scale.y - var_borderThickness * 2.0;

	float centerX = var_scale.x * 0.5;
	float centerY = var_scale.y * 0.5;

	float borderX = abs(centerX - var_passVertexPos.x) * 2.0;
	float borderY = abs(centerY - var_passVertexPos.y) * 2.0;

	if (borderX < borderThresholdX && borderY < borderThresholdY)
		outColor = texture(tex, var_texCoord) * var_color;
	else
		outColor = var_borderColor;
}