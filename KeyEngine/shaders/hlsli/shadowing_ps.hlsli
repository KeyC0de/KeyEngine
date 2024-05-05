// Omnidirectional (PointLight) Cube Texture for Shadow mapping
TextureCube shadowCubeMap : register(t3);
SamplerComparisonState shadowCubeMapSampler : register(s1);

float calculateShadowCubeMapSampleVectorProjectionLength( const in float3 fragPosLightSpace )
{
	static const float zf = 100.0f;
	static const float zn = 1.0f;
	static const float c0 = -zn * zf / ( zf - zn );
	static const float c1 = zf / ( zf - zn );

	// get magnitudes for each component
	const float3 sampleVectorMags = abs( fragPosLightSpace);
	// now we find out which is the major axis that corresponds to the face of the shadow cube map
	const float dominantAxisLen = max( sampleVectorMags.x, max( sampleVectorMags.y, sampleVectorMags.z ) );
	// project the light-space-fragment to the plane/face of the cube
	return c0 / dominantAxisLen + c1;
}

// the Comparison Sampler is used here to filter out fragments that rest inside the light space
// this will be the case if the position of the fragment indicated by the sample vector's length is less than 1.0f
// so if that fragment is not visible to the light ie when:
// ( input.fragPosLightSpace.x < -1.0f || input.fragPosLightSpace.x > 1.0f ||
//		input.fragPosLightSpace.y < -1.0f || input.fragPosLightSpace.y > 1.0f ||
//		input.fragPosLightSpace.z < 0.0f  || input.fragPosLightSpace.z > 1.0f )
// we will not illuminate it and we will apply the appropriate shadow level
float calculateShadowLevel( const in float4 fragPosLightSpace )
{
	return shadowCubeMap.SampleCmpLevelZero( shadowCubeMapSampler, fragPosLightSpace.xyz, calculateShadowCubeMapSampleVectorProjectionLength( fragPosLightSpace.xyz ) );
}