cbuffer SpotLightPSCB : register(b3)
{
	float3 spotLightPosViewSpace;
	float3 ambient;
	float3 lightColor;
	float intensity;
	float attConstant;
	float attLinear;
	float attQuadratic;
	float3 spotLightDirViewSpace;
	float coneAngle;
};