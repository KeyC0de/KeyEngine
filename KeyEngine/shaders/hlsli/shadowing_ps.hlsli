// Omnidirectional (PointLight) Cube Texture for Shadow mapping
TextureCube shadowCubeMap : register(t3);
SamplerComparisonState shadowCubeMapSampler : register(s1);

static const float zf = 100.0f;
static const float zn = 1.0f;
static const float c0 = -zn * zf / ( zf - zn );
static const float c1 = zf / ( zf - zn );

float calculateShadowCubeMapSampleVectorProjectionLength( const in float4 fragPosLightSpace )
{
	// get magnitudes for each component
	const float3 sampleVectorMags = abs( fragPosLightSpace ).xyz;
	// now we find out which is the major axis that corresponds to the face of the shadow cube map
	const float dominantAxisLen = max( sampleVectorMags.x, max( sampleVectorMags.y, sampleVectorMags.z ) );
	// get the projection length (projection forward direction for the cube face we want)
	return c0 / dominantAxisLen + c1;
}

// if position is not visible to the light don't illuminate it
// if ( input.fragPosLightSpace.x < -1.0f || input.fragPosLightSpace.x > 1.0f ||
//		input.fragPosLightSpace.y < -1.0f || input.fragPosLightSpace.y > 1.0f ||
//		input.fragPosLightSpace.z < 0.0f  || input.fragPosLightSpace.z > 1.0f )
float calculateShadowLevel( const in float4 fragPosLightSpace )
{
	return shadowCubeMap.SampleCmpLevelZero( shadowCubeMapSampler, fragPosLightSpace.xyz, calculateShadowCubeMapSampleVectorProjectionLength( fragPosLightSpace ) );
}