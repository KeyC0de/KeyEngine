////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief lighting utilities

struct LightVectors
{
	float3 L;	/// \brief	the incident light vector from the fragment's position to the light's position
	float3 LNormalized;
	float lengthOfL;
};

LightVectors calculatePointLightVectors( const in float3 lightPositionViewSpace,
	const in float3 fragmentPositionViewSpace )
{
	LightVectors lv;
	lv.L = lightPositionViewSpace - fragmentPositionViewSpace;
	lv.lengthOfL = length( lv.L );
	lv.LNormalized = lv.L / lv.lengthOfL;
	return lv;
}

float calculateLightAttenuation( const in float fragmentToLightDistance,
	uniform float attConstant,
	uniform float attLinear,
	uniform float attQuadratic )
{
	return 1.0f / ( attConstant + attLinear * fragmentToLightDistance + attQuadratic * (fragmentToLightDistance * fragmentToLightDistance) );
}

float3 calculateLightDiffuseContribution( uniform float3 lightColor,
	uniform float intensity,
	const in float attenuation,
	const in float3 fragmentToLightDirViewSpaceNormalized,
	const in float3 viewSpaceNormalNormalized )
{
	return attenuation * lightColor * intensity * max( 0.0f, dot( fragmentToLightDirViewSpaceNormalized, viewSpaceNormalNormalized ) );
}

float3 calculateLightSpecularContribution( const in float3 lightColor,
	const in float3 specularColor,
	uniform float intensity,
	const in float fragSpecularGloss,
	const in float3 viewSpaceNormalNormalized,
	const in float3 fragmentToLightDirViewSpace,	// light direction vector in view space coords
	const in float3 viewSpacePos,
	const in float attenuation )
{
	const float3 specularReflectionViewSpace = 2.0f * viewSpaceNormalNormalized * dot( fragmentToLightDirViewSpace, viewSpaceNormalNormalized ) - fragmentToLightDirViewSpace;
	const float3 specularReflectionViewSpaceNormalized = normalize( specularReflectionViewSpace );
	const float3 viewSpacePosNormalized = normalize( viewSpacePos );
	return attenuation * lightColor * intensity * specularColor * pow( max( 0.0f, dot( -specularReflectionViewSpaceNormalized, viewSpacePosNormalized ) ), fragSpecularGloss );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief texturing utilities

float3 calculateNormalMapNormal( const in float3 tangentViewSpace,
	const in float3 bitangentViewSpace,
	const in float3 viewSpaceNormalNormalized,
	const in float2 tc,
	uniform Texture2D normalMap,
	uniform SamplerState sampl )
{
	// build the TBN rotation matrix
	const float3x3 tbn = float3x3( tangentViewSpace, bitangentViewSpace, viewSpaceNormalNormalized );
	// sample normal map
	const float3 normalMapNormal = normalMap.Sample( sampl, tc ).xyz;
	// convert normal ranges from uv space[0,1] into 3d space[-1,1]
	const float3 tangentSpaceNormal = normalMapNormal * 2.0f - 1.0f;
	// convert normal map normals from tangent space into normal (clip) space
	return normalize( mul( tangentSpaceNormal, tbn ) );
}
