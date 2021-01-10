#version 330 core

in vec2 var_uv;
in float var_diffuseFactor;
in vec4 var_shadowCoord;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_shadowmap;

uniform int isTwoSided;

uniform vec3 directionalLight_ambientColor;
uniform vec3 directionalLight_color;
const float shadowStrength = 0.4;

struct ShadowProperties
{
	int shadowmapWidth;
	int pcfCount; // This is actually half pcf count - 1
};

uniform ShadowProperties shadowProperties;

layout(location = 0) out vec4 outColor;

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
	int texelCount = texelsCount_width * texelsCount_width;
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
	float bias = 0.005f; // max(0.025 * 1.0 - dotp_normToLight, 0.005);
	float shadow = min(calcShadow(bias), shadowStrength);

	//float diffuseFactor = max(dot(toLightVec, vec3(0,1,0)) - shadow, 0.0);
	float diffuseFactor = var_diffuseFactor;
	vec4 texColor_diffuse =	texture(texture_diffuse, var_uv);
	
	vec4 ambient =	vec4(directionalLight_ambientColor, 1) * texColor_diffuse;
	vec4 diffuse =	vec4(directionalLight_color, 1) * diffuseFactor * texColor_diffuse;
	
	outColor = (ambient + diffuse) - vec4(shadow, shadow, shadow, 0.0);
	if (outColor.a < 0.5) discard;
}