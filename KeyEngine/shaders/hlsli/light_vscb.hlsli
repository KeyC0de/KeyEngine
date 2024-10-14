#define MAX_LIGHTS 16


// cb_lightMatrix performs a coordinate transformation whose origin is at the Light's origin location
cbuffer LightVSCB : register(b2)
{
	matrix cb_lightMatrix[MAX_LIGHTS];
};

float4 convertVertexPosToNonPointLightSpace( const in float4 pos,
	uniform matrix worldMatrix,
	const in int lightIndex )
{
	const float4 worldSpacePos = mul( pos, worldMatrix );
	return mul( worldSpacePos, cb_lightMatrix[lightIndex] );
	// alternatively consider doing cb_world.xyz - lightPosition.xyz, which effectively transforms the vertex position to light space
}

float4 convertVertexPosToPointLightSpace( const in float4 pos,
	uniform matrix worldMatrix,
	const in int lightIndex )
{
	const float4 worldSpacePos = mul( pos, worldMatrix );
	return mul( worldSpacePos, cb_lightMatrix[lightIndex] );
	// alternatively consider doing cb_world.xyz - lightPosition.xyz, which effectively transforms the vertex position to light space
}