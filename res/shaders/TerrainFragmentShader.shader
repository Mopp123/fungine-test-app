#version 330 core

in vec2 var_uv;
in vec3 var_fragPos;
in vec3 var_normal;
in vec3 var_toCameraVec;
in vec3 var_dirLightDirection;
in mat3 var_tbnMatrix;

uniform sampler2D texture_blendmap;

uniform sampler2D texture_black_diffuse;
uniform sampler2D texture_black_specular;
uniform sampler2D texture_black_normal;

uniform sampler2D texture_red_diffuse;
uniform sampler2D texture_red_specular;
uniform sampler2D texture_red_normal;

uniform sampler2D texture_green_diffuse;
uniform sampler2D texture_green_specular;
uniform sampler2D texture_green_normal;

uniform sampler2D texture_blue_diffuse;
uniform sampler2D texture_blue_specular;
uniform sampler2D texture_blue_normal;


uniform vec3 directionalLight_ambientColor;
uniform vec3 directionalLight_color;

const float specularStrength = 1.0;
const float specularShininess = 64.0;

layout(location = 0) out vec4 outColor;

/*
	Its not enough to normalize vector in vertex shader and send it here!
	You would neet to normalize that again here, since some floating point and percicion issues (or in some cases just because of the interpolation)?
*/

const float textureTiling = 20.0;
void calcTextureColors(out vec4 diffuseColor, out vec4 specularColor, out vec4 normalColor)
{
	vec4 blendmapColor = texture(texture_blendmap, var_uv);
	float blackAmount = 1.0 - (blendmapColor.r + blendmapColor.g + blendmapColor.b);
	vec2 tiledUv = var_uv * textureTiling;

	vec4 color_diffuse_black =	texture(texture_black_diffuse, tiledUv) * blackAmount;
	vec4 color_diffuse_red =	texture(texture_red_diffuse, tiledUv) * blendmapColor.r;
	vec4 color_diffuse_green =	texture(texture_green_diffuse, tiledUv) * blendmapColor.g;
	vec4 color_diffuse_blue =	texture(texture_blue_diffuse, tiledUv) * blendmapColor.b;

	vec4 color_specular_black = texture(texture_black_specular, tiledUv) * blackAmount;
	vec4 color_specular_red =	texture(texture_red_specular, tiledUv) * blendmapColor.r;
	vec4 color_specular_green = texture(texture_green_specular, tiledUv) * blendmapColor.g;
	vec4 color_specular_blue =	texture(texture_blue_specular, tiledUv) * blendmapColor.b;

	vec4 color_normal_black =	texture(texture_black_normal, tiledUv) * blackAmount;
	vec4 color_normal_red =		texture(texture_red_normal, tiledUv) * blendmapColor.r;
	vec4 color_normal_green =	texture(texture_green_normal, tiledUv) * blendmapColor.g;
	vec4 color_normal_blue =	texture(texture_blue_normal, tiledUv) * blendmapColor.b;

	diffuseColor = color_diffuse_black + color_diffuse_red + color_diffuse_green + color_diffuse_blue;
	specularColor = color_specular_black + color_specular_red + color_specular_green + color_specular_blue;
	normalColor = color_normal_black + color_normal_red + color_normal_green + color_normal_blue;
}

void main(void)
{
	vec4 texColor_diffuse = vec4(0, 0, 0, 1);
	vec4 texColor_specular = vec4(0, 0, 0, 1);
	vec4 texColor_normal = vec4(0, 0, 0, 1);
	calcTextureColors(texColor_diffuse, texColor_specular, texColor_normal);

	vec3 normal = normalize(texColor_normal.rgb * 2.0 - 1.0);

	vec3 toLightVec = normalize(-var_dirLightDirection); // not sure is this correct for this directional light?
	vec3 toCameraVec = normalize(var_toCameraVec);
	float diffuseFactor = max(dot(toLightVec, normal), 0.0);

	// *Note: even if you thought these 2 inputted vectors for reflect() were unit vectors, this may not be true, since floating point issues..
	vec3 reflectedLightVec = normalize(reflect(var_dirLightDirection, normal));
	float specularFactor = specularStrength * pow(max(dot(reflectedLightVec, toCameraVec), 0.0), specularShininess);

	vec4 ambient =	vec4(directionalLight_ambientColor, 1) * texColor_diffuse;
	vec4 diffuse =	vec4(directionalLight_color, 1) * diffuseFactor * texColor_diffuse;
	vec4 specular = vec4(directionalLight_color, 1) * specularFactor * texColor_specular;

	outColor = (ambient + diffuse + specular);
}