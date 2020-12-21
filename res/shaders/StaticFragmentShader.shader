#version 330 core

in vec2 var_uv;
in vec3 var_fragPos;
in vec3 var_normal;
in vec3 var_toCameraVec;
in vec3 var_dirLightDirection;
in mat3 var_tbnMatrix;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform sampler2D texture_normal;

uniform vec3 directionalLight_ambientColor;
uniform vec3 directionalLight_color;

const float specularStrength = 1.0;
const float specularShininess = 64.0;

layout(location = 0) out vec4 outColor;

/*
	Its not enough to normalize vector in vertex shader and send it here!
	You would neet to normalize that again here, since some floating point and percicion issues (or in some cases just because of the interpolation)?
*/

void main(void)
{
	vec3 normal = normalize(texture(texture_normal, var_uv).rgb * 2.0 - 1.0);

	vec3 toLightVec = normalize(-var_dirLightDirection); // not sure is this correct for this directional light?
	vec3 toCameraVec = normalize(var_toCameraVec);
	float diffuseFactor = max(dot(toLightVec, normal), 0.0);

	// *Note: even if you thought these 2 inputted vectors for reflect() were unit vectors, this may not be true, since floating point issues..
	vec3 reflectedLightVec = normalize(reflect(var_dirLightDirection, normal));
	float specularFactor = specularStrength * pow(max(dot(reflectedLightVec, toCameraVec), 0.0), specularShininess);

	vec4 texColor_diffuse =		texture(texture_diffuse, var_uv);
	vec4 texColor_specular =	texture(texture_specular, var_uv);
	
	vec4 ambient =	vec4(directionalLight_ambientColor, 1) * texColor_diffuse;
	vec4 diffuse =	vec4(directionalLight_color, 1) * diffuseFactor * texColor_diffuse;
	vec4 specular = vec4(directionalLight_color, 1) * specularFactor * texColor_specular;

	outColor = (ambient + diffuse + specular);
	if (outColor.a < 0.5) discard;
}