cbuffer DirectionalLightPSCB : register(b1)
{
	float3 directionalLightDirViewSpace;
	float3 ambient;
	float3 lightColor;
	float intensity;
};