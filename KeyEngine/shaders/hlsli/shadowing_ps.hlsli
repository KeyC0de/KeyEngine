#define MAX_LIGHTS 16


/// \brief Omnidirectional (PointLight) Cube Texture for Shadow mapping
//TextureCube shadowCubeMap : register(t4);
//Texture2DArray shadowMapArray : register(t3);
TextureCubeArray shadowCubeMapArray : register(t4);
SamplerComparisonState shadowMapSampler : register(s1);

/// \brief sampling the depth from the appropriate face of the shadow cube map
float calculateShadowCubeMapSampleVectorProjectionLength( const in float3 fragPosLightSpace,
	const in float shadowCamNearZ,
	const in float shadowCamFarZ )
{
	const float A = -shadowCamFarZ * shadowCamNearZ / ( shadowCamFarZ - shadowCamNearZ );
	const float B = shadowCamFarZ / ( shadowCamFarZ - shadowCamNearZ );

	// get magnitudes for each component
	const float3 sampleVectorMags = abs( fragPosLightSpace );
	// now we find out which is the major axis that corresponds to the face of the shadow cube map
	// the dominant component determines the face of the cube we sample from
	const float dominantAxisLen = max( sampleVectorMags.x, max( sampleVectorMags.y, sampleVectorMags.z ) );
	// once the dominant component (of the uvw coordinate) is found then the other 2 components are used to compute the uv coordinates on that face
	// project the light space depth-coord (whichever one may be the dominant one, ie. x, y or z) into the corresponding face of the cube
	return A / dominantAxisLen + B;	// f * ( 1 - n * dominantAxisLen ) / ( f - n );

	// The .x, .y, and .z components of fragPosLightSpace are not UVW coordinates by themselves.
	// Instead they are manipulated in the above manner to determine the UVW coordinates
	// example: Direction Vector (1.2, -0.5, 0.2):
	// Face Selection: Here, 1.2 is the largest absolute value, so the positive X face (+X) is selected.
	// 2D Coordinates: The Y and Z components (-0.5, 0.2) are used to compute the 2D coordinates on the +X face.
	//		you could think of -0.5 and 0.2 as the uv coordinates that will map on the +x face
}

/// \brief the Comparison Sampler is used here to filter out fragments that rest outside the light space
/// \brief this will be the case if the position of the fragment indicated by the sample vector's length is less than 1.0f
/// \brief so if that fragment is not visible to the light ie when:
/// \brief ( input.fragPosLightSpace.x < -1.0f || input.fragPosLightSpace.x > 1.0f ||
/// \brief		input.fragPosLightSpace.y < -1.0f || input.fragPosLightSpace.y > 1.0f ||
/// \brief		input.fragPosLightSpace.z < 0.0f  || input.fragPosLightSpace.z > 1.0f )
/// \brief we will not illuminate it and we will apply the appropriate shadow level
/// \brief using TextureCubeArray for now to accommodate multiple point lights
/// \brief	calculateShadowLevelCubeMap( const in float4 fragPosLightSpace )
/// \brief	{
/// \brief		return shadowCubeMap.SampleCmpLevelZero( shadowMapSampler, fragPosLightSpace.xyz, calculateShadowCubeMapSampleVectorProjectionLength( fragPosLightSpace.xyz ) ).x;
/// \brief	}

float calculateShadowLevelCubeMapArray( const in float4 fragPosLightSpace,
	const in float arrayIndex,
	const in float shadowCamNearZ,
	const in float shadowCamFarZ )
{
	return shadowCubeMapArray.SampleCmpLevelZero( shadowMapSampler, float4(fragPosLightSpace.xyz, arrayIndex), calculateShadowCubeMapSampleVectorProjectionLength( fragPosLightSpace.xyz, shadowCamNearZ, shadowCamFarZ ) ).x;
}

/*
float calculateShadowLevelMapArray( const in float2 uv,
	const in float arrayIndex )
{
	return shadowMapArray.Sample( shadowMapSampler, float3(uv, arrayIndex) ).x;
}
*/