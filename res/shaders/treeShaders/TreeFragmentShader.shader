#version 330 core

in vec2 var_uv;
in vec3 var_fragPos;
in vec3 var_normal;
in vec3 var_toCameraVec;
in vec3 var_dirLightDirection;
in mat3 var_tbnMatrix;
in vec4 var_shadowCoord;

uniform sampler2D m_texture_diffuse;
uniform sampler2D m_texture_specular;
uniform sampler2D m_texture_normal;
uniform sampler2D texture_shadowmap;

uniform vec3 directionalLight_ambientColor;
uniform vec3 directionalLight_color;

struct ShadowProperties
{
	int shadowmapWidth;
	int pcfCount; // This is actually half pcf count - 1
};

uniform ShadowProperties shadowProperties;

const float specularStrength = 1.0;
const float specularShininess = 64.0;

layout(location = 0) out vec4 outColor;

/*
	Its not enough to normalize vector in vertex shader and send it here!
	You would neet to normalize that again here, since some floating point and percicion issues (or in some cases just because of the interpolation)?
*/
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
	vec3 normal = normalize(texture(m_texture_normal, var_uv).rgb * 2.0 - 1.0);

	vec3 toLightVec = normalize(-var_dirLightDirection); // not sure is this correct for this directional light?
	vec3 toCameraVec = normalize(var_toCameraVec);
	
	float dotp_normToLight = dot(toLightVec, normal);
	float diffuseFactor = 0.0;

	diffuseFactor = max(dotp_normToLight, 0.0);
	

	float bias = max(0.025 * 1.0 - dotp_normToLight, 0.005);
	float shadow = calcShadow(bias);

	// *Note: even if you thought these 2 inputted vectors for reflect() were unit vectors, this may not be true, since floating point issues..
	vec3 reflectedLightVec = normalize(reflect(var_dirLightDirection, normal));
	float specularFactor = specularStrength * pow(max(dot(reflectedLightVec, toCameraVec), 0.0), specularShininess);

	vec4 texColor_diffuse =		texture(m_texture_diffuse, var_uv);
	vec4 texColor_specular =	texture(m_texture_specular, var_uv);
	
	vec4 ambient =	vec4(directionalLight_ambientColor, 1) * texColor_diffuse;
	vec4 diffuse =	vec4(directionalLight_color, 1) * diffuseFactor * texColor_diffuse;
	vec4 specular = vec4(directionalLight_color, 1) * specularFactor * texColor_specular;

	outColor = (ambient + diffuse + specular) - vec4(shadow, shadow, shadow, 0.0);
	if (outColor.a < 0.5) discard;
}