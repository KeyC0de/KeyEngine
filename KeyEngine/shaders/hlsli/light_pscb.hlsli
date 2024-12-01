#define MAX_LIGHTS 16


struct LightProperties
{
	int cb_lightType;				// 1 for directional lights, 2 for spot lights, 3 for point lights
	int cb_bCastingShadows;
	float2 padding;
	float3 cb_lightPosViewSpace;	// represents direction for Directional Lights
	float3 cb_lightColor;			// albedo/diffuse light color
	float cb_intensity;
	float cb_attConstant;
	float cb_attLinear;
	float cb_attQuadratic;
	float3 cb_spotLightDirViewSpace;
	float cb_coneAngle;
};

cbuffer LightPSCB : register(b2)
{
	LightProperties cb_lights[MAX_LIGHTS];
};

static const float3 g_ambientColor = float3(0.08f, 0.08f, 0.08f);
