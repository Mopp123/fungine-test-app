#version 330 core

in vec2 var_texCoord;

uniform sampler2D fontTexture;
uniform vec4 fontColor;

out vec4 outColor;

void main(void)
{
	vec4 textureColor = texture(fontTexture, var_texCoord);
	float brightness = textureColor.r;
	
	if (brightness == 0)
		discard;

	outColor = vec4(fontColor.r, fontColor.g, fontColor.b, brightness);

}