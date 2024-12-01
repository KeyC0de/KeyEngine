struct PointLightVectors
{
	float3 vToL;
	float3 vToL_normalized;
	float lengthOfL;
};

PointLightVectors calculatePointLightVectors( const in float3 lightPositionViewSpace,
	const in float3 fragmentPositionViewSpace )
{
	PointLightVectors plv;
	plv.vToL = lightPositionViewSpace - fragmentPositionViewSpace;
	plv.lengthOfL = length( plv.vToL );
	plv.vToL_normalized = plv.vToL / plv.lengthOfL;
	return plv;
}

float calculateLightAttenuation( const in float fragmentToLightDistance,
	uniform float attConstant,
	uniform float attLinear,
	uniform float attQuadratic )
{
	return 1.0f / ( attConstant + attLinear * fragmentToLightDistance + attQuadratic * (fragmentToLightDistance * fragmentToLightDistance) );
}

float3 calculateLightDiffuseContribution( uniform float3 diffuseColor,
	uniform float intensity,
	const in float attenuation,
	const in float3 fragmentToLightDirViewSpaceNormalized,
	const in float3 viewSpaceNormal )
{
	return attenuation * diffuseColor * intensity * max( 0.0f, dot( fragmentToLightDirViewSpaceNormalized, viewSpaceNormal ) );
}

float3 calculateLightSpecularContribution( const in float3 cb_lightColor,
	const in float3 specularColor,
	uniform float intensity,
	const in float fragSpecularGloss,
	const in float3 viewSpaceNormal,
	const in float3 fragmentToLightDirViewSpace,
	const in float3 viewSpacePos,
	const in float attenuation )
{
	const float3 specularReflection = normalize( 2.0f * viewSpaceNormal * dot( fragmentToLightDirViewSpace, viewSpaceNormal ) - fragmentToLightDirViewSpace );
	const float3 viewSpacePosNormalized = normalize( viewSpacePos );
	// calculate specular component color based on angle between
	// viewing vector and reflection vector - narrow with power function
	return cb_lightColor * specularColor * intensity * attenuation * pow( max( 0.0f, dot( -specularReflection, viewSpacePosNormalized ) ), fragSpecularGloss );
}

float3 calculateNormalMapNormal( const in float3 tangentViewSpace,
	const in float3 bitangentViewSpace,
	const in float3 viewSpaceNormal,
	const in float2 tc,
	uniform Texture2D normalMap,
	uniform SamplerState sampl )
{
	// build the TBN rotation matrix
	const float3x3 tbn = float3x3( tangentViewSpace, bitangentViewSpace, viewSpaceNormal);
	// sample normal map
	const float3 normalMapNormal = normalMap.Sample( sampl, tc ).xyz;
	// convert normal ranges from uv space[0,1] into 3d space[-1,1]
	const float3 tangentSpaceNormal = normalMapNormal * 2.0f - 1.0f;
	// convert normal map normals from tangent space into normal (clip) space
	return normalize( mul( tangentSpaceNormal, tbn ) );
}