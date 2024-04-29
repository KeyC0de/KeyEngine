cbuffer PointLightSourceShadowTransformVSCB : register(b1)
{
	matrix lightMatrix;
};

float4 convertVertexPosToLightSpace( const in float3 pos,
	uniform matrix modelTransform )
{
	const float4 world = mul( float4( pos, 1.0f ), modelTransform );
	return mul( world, lightMatrix );
}