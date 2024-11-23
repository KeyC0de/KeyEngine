#define MAX_LIGHTS 16


// cb_lightMatrix performs a coordinate transformation whose origin is at the Light's origin location
cbuffer LightVSCB : register(b2)
{
	matrix cb_lightMatrix[MAX_LIGHTS];
};

// Directional and Spotlight Shadows: The pixel shader samples the shadow map using the transformed light-space position.
float4 convertVertexPosToNonPointLightSpace( const in float4 vertexPos,
	uniform matrix worldMatrix,
	const in int lightIndex )
{
	const float4 worldSpacePos = mul( vertexPos, worldMatrix );
	return mul( worldSpacePos, cb_lightMatrix[lightIndex] );
	// alternatively consider doing cb_world.xyz - lightPosition.xyz, which effectively transforms the vertex position to light space
}

// Point Light Shadows: The PS calculates the direction to the point light and performs a cubemap lookup using the world-space position.
float4 convertVertexPosToPointLightSpace( const in float4 vertexPos,
	uniform matrix worldMatrix,
	const in int lightIndex )
{
	const float4 worldSpacePos = mul( vertexPos, worldMatrix );
	return mul( worldSpacePos, cb_lightMatrix[lightIndex] );
	// alternatively consider return worldSpacePos
	// alternatively consider doing cb_world.xyz - lightPosition.xyz, which effectively transforms the vertex position to light space
}