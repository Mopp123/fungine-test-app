#version 330 core

in vec2 var_uv;
in vec3 var_normal;
in vec3 var_toCameraVec;
in vec3 var_dirLightDirection;

uniform sampler2D texture_diffuse;

uniform int isTwoSided;

uniform vec3 directionalLight_ambientColor;
uniform vec3 directionalLight_color;

layout(location = 0) out vec4 outColor;

/*
	Its not enough to normalize vector in vertex shader and send it here!
	You would neet to normalize that again here, since some floating point and percicion issues (or in some cases just because of the interpolation)?
*/

void main(void)
{
	vec3 normal = normalize(var_normal);

	vec3 toLightVec = normalize(-var_dirLightDirection); // not sure is this correct for this directional light?
	vec3 toCameraVec = normalize(var_toCameraVec);
	
	if(isTwoSided == 1)
	{
		float dotp_norm_toLight = dot(toLightVec, normal);
		if (dotp_norm_toLight < 0.0)
			normal = -normal;
	}

	float diffuseFactor = max(dot(toLightVec, normal), 0.0);


	vec4 texColor_diffuse =	texture(texture_diffuse, var_uv);
	
	vec4 ambient =	vec4(directionalLight_ambientColor, 1) * texColor_diffuse;
	vec4 diffuse =	vec4(directionalLight_color, 1) * diffuseFactor * texColor_diffuse;
	
	outColor = (ambient + diffuse);
	if (outColor.a < 0.5) discard;
}