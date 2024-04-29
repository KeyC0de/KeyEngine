cbuffer PointLightPSCB : register(b2)
{
	float3 pointLightPosViewSpace;
	float3 ambient;
	float3 lightColor;		// diffuse color
	float intensity;
	float attConstant;
	float attLinear;
	float attQuadratic;
};