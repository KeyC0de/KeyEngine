#define MAX_LIGHTS 16


struct LightProperties
{
	int cb_lightType;		// 1 for directional lights, 2 for point lights, 3 for spot lights
	int cb_bCastingShadows;
	float2 padding;
	float3 cb_lightPosViewSpace;
	float3 ambient;
	float3 lightColor;		// diffuse color
	float intensity;
	float attConstant;
	float attLinear;
	float attQuadratic;
	float3 cb_spotLightDirViewSpace;
	float cb_coneAngle;
};

cbuffer LightPSCB : register(b2)
{
	LightProperties cb_lights[MAX_LIGHTS];
};