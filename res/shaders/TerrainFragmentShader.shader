#version 330 core

in vec2 var_uv;
in vec3 var_fragPos;
in vec3 var_normal;
in vec3 var_toCameraVec;
in vec3 var_dirLightDirection;
in mat3 var_tbnMatrix;
in vec4 var_shadowCoord;

uniform sampler2D m_texture_blendmap;

uniform sampler2D m_texture_black_diffuse;
uniform sampler2D m_texture_black_specular;
uniform sampler2D m_texture_black_normal;

uniform sampler2D m_texture_red_diffuse;
uniform sampler2D m_texture_red_specular;
uniform sampler2D m_texture_red_normal;

uniform sampler2D m_texture_green_diffuse;
uniform sampler2D m_texture_green_specular;
uniform sampler2D m_texture_green_normal;

uniform sampler2D m_texture_blue_diffuse;
uniform sampler2D m_texture_blue_specular;
uniform sampler2D m_texture_blue_normal;

uniform sampler2D texture_shadowmap;


uniform vec3 directionalLight_ambientColor;
uniform vec3 directionalLight_color;

const float specularStrength = 0.5;
const float specularShininess = 32.0;

struct ShadowProperties
{
	int shadowmapWidth;
	int pcfCount; // This is actually half pcf count - 1
};

uniform ShadowProperties shadowProperties;

layout(location = 0) out vec4 outColor;

/*
	Its not enough to normalize vector in vertex shader and send it here!
	You would neet to normalize that again here, since some floating point and percicion issues (or in some cases just because of the interpolation)?
*/

const float textureTiling = 20.0;
void calcTextureColors(out vec4 diffuseColor, out vec4 specularColor, out vec4 normalColor)
{
	vec4 blendmapColor = texture(m_texture_blendmap, var_uv);
	float blackAmount = 1.0 - (blendmapColor.r + blendmapColor.g + blendmapColor.b);
	vec2 tiledUv = var_uv * textureTiling;

	vec4 color_diffuse_black =	texture(m_texture_black_diffuse, tiledUv) * blackAmount;
	vec4 color_diffuse_red =	texture(m_texture_red_diffuse, tiledUv) * blendmapColor.r;
	vec4 color_diffuse_green =	texture(m_texture_green_diffuse, tiledUv) * blendmapColor.g;
	vec4 color_diffuse_blue =	texture(m_texture_blue_diffuse, tiledUv) * blendmapColor.b;

	vec4 color_specular_black = texture(m_texture_black_specular, tiledUv) * blackAmount;
	vec4 color_specular_red =	texture(m_texture_red_specular, tiledUv) * blendmapColor.r;
	vec4 color_specular_green = texture(m_texture_green_specular, tiledUv) * blendmapColor.g;
	vec4 color_specular_blue =	texture(m_texture_blue_specular, tiledUv) * blendmapColor.b;

	vec4 color_normal_black =	texture(m_texture_black_normal, tiledUv) * blackAmount;
	vec4 color_normal_red =		texture(m_texture_red_normal, tiledUv) * blendmapColor.r;
	vec4 color_normal_green =	texture(m_texture_green_normal, tiledUv) * blendmapColor.g;
	vec4 color_normal_blue =	texture(m_texture_blue_normal, tiledUv) * blendmapColor.b;

	diffuseColor = color_diffuse_black + color_diffuse_red + color_diffuse_green + color_diffuse_blue;
	specularColor = color_specular_black + color_specular_red + color_specular_green + color_specular_blue;
	normalColor = color_normal_black + color_normal_red + color_normal_green + color_normal_blue;
}


/*	
	Tests "early bail" for shadow pcf filtering. If early bail possible returns 1
	Testing is performed by comparing conrners of "pcf area" to the center texel

	If early bail success, stores final shadow into "out float shadowVal"
*/
int shadowPCF_earlyBailTest(float bias, int pcfCount, vec2 texelSize, out float shadowVal)
{
	int testCount = 0;
	float shadowmapCenter = texture(texture_shadowmap, var_shadowCoord.xy * texelSize).r;

	for (int x = -pcfCount; x <= pcfCount; x += pcfCount * 2)
	{
		for (int y = -pcfCount; y <= pcfCount; y += pcfCount * 2)
		{
			float d = texture(texture_shadowmap, var_shadowCoord.xy + vec2(x, y) * texelSize).r;
			// Store the last shadow val, if early bail succeeds, we can use this..
			shadowVal = var_shadowCoord.z - bias > d ? 1.0 : 0.0;

			if (d == shadowmapCenter)
				testCount++;
		}
	}

	if (testCount == 4)
	{
		if (var_shadowCoord.z > 1.0)
			shadowVal = 0.0;

		return 1;
	}
	else
	{
		shadowVal = 0.0;
		return 0;
	}
}

float calcShadow(float bias)
{
	float shadow = 0.0;
	int shadowmapWidth = shadowProperties.shadowmapWidth;
	int pcfCount = shadowProperties.pcfCount;
	int texelsCount_width = (2 * pcfCount + 1);
	int texelCount =  texelsCount_width * texelsCount_width;
	vec2 texelSize = 1.0 / vec2(shadowmapWidth, shadowmapWidth);
	
	// "early bail" testing
	if (shadowPCF_earlyBailTest(bias, pcfCount, texelSize, shadow) == 0)
	{
		// Shadow pcf filtering
		for (int x = -pcfCount; x <= pcfCount; x++)
		{
			for (int y = -pcfCount; y <= pcfCount; y++)
			{
				float d = texture(texture_shadowmap, var_shadowCoord.xy + vec2(x, y) * texelSize).r;
				shadow += var_shadowCoord.z - bias > d ? 1.0 : 0.0;
			}
		}
		shadow /= texelCount;

		// that weird far plane shadow
		if (var_shadowCoord.z > 1.0)
			return 0.0;
	}
	return shadow;
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
	
	float dopt_normToLight = dot(toLightVec, normal);

	// Shadows
	float bias = 0.005;//max(0.025 * (1.0 - dopt_normToLight), 0.005);
	float shadow = calcShadow(bias);
	
	float diffuseFactor = max(dopt_normToLight - shadow, 0.0);
	
	// *Note: even if you thought these 2 inputted vectors for reflect() were unit vectors, this may not be true, since floating point issues..
	vec3 reflectedLightVec = normalize(reflect(var_dirLightDirection, normal));
	float specularFactor = specularStrength * pow(max(dot(reflectedLightVec, toCameraVec), 0.0), specularShininess);


	vec4 ambient =	vec4(directionalLight_ambientColor, 1) * texColor_diffuse;
	vec4 diffuse =	vec4(directionalLight_color, 1) * diffuseFactor * texColor_diffuse;
	vec4 specular = vec4(directionalLight_color, 1) * specularFactor * texColor_specular;

	outColor = ambient + diffuse + specular;
}