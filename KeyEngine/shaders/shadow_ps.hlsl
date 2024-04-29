// PS Journal

// Shadow Map Pass VS
//struct Output
//{
//    float4 pos : SV_Position;	// will be SSDBed
//	// we will compare these 2 values in the PS and we will see how the SSDB
//	float4 viewSpacePos : Position; // no biased - output to PS - we used that initially
//	// to test what circuitry the SSDB was doing on `pos` by comparing the values on PS
//	// (also taking SV_Position on PS) and then we can apply the operation here
//	// to test that it's done correctly and you get the right result in the PS you can:
//	// use float4 viewSpacePos
//	// viewSpacePos.w = pos.z; // not: pos.z / pos.w; it wouldn't work like that by doing division in the VS
//	// then in the PS:
//	// return float2(pos.z, viewSpacePos.w / pos.w ); // not: float2(pos.z, viewSpacePos.w)	// should be the same
//	// we check the differences in python with numpy by subtracting the images and doing min, max std etc. to check that the differences are minimally low (attributed only to floating point error)
//};

//Output main(float3 pos : Position)
//{
//    Output output;
//	output.pos = mul(float4(pos, 1.0f), worldViewProjection);
//	// get vertex position relative to light source (oriented relative to current shadow)
//	output.viewSpacePos.xyz = mul(float4(pos, 1.0f), worldView).xyz;
//	// output.viewSpacePos.w = output.pos.z / output.pos.w;	// don't divide in the VS, but in the PS
//	output.viewSpacePos.w = output.pos.z;
//
//	// store z as cartesian distance to make cubemap depth comparison convenient & fast
//	// st we don't have to know which direction the point is in (which one of the 6 faces it corresponds to)
//	//		but only the scalar distance
//	// depth stored will be linear actual distance from point light (fp is locked to 100)
//	// also premultiply by w to counteract the perspective division of z
//	//output.pos.z = length( output.viewSpacePos ) * output.pos.w / 100.0f;
//	// Removed from the VS!
//	// 1. We made the grave mistake of interpolating a non linear function (length, which is not linear = sqrt(x^2 + y^2 + z^2)
//	//	and outputting it into the pixel shader - when in our other Pixel Shaders (eg. Phong_PS) we make use of x, y, z values
//	// 2. Also for Slope Scale depth biasing to work, we need the Z as is in the vertex shader
//	//
//	// Instead we can calculate the length between the current pixel and the target's depth it in the PS
//	//		and we will Shadow_PS.hlsl
//	//
//	// Also note we don't want to manipulate depth (SV_Depth) in the PS because it disables a lot of
//	//	z optimizations including early z culling
//
//	return output;
//}
//

// PS
// No need for Pixel Shader. The ShadowMap pass only needs to output depth information.
struct PSIn
{
	float4 pos : SV_Position;	// SV_Position in the PS provides NDC space values
	float4 viewSpacePos : Position;
};

struct PSOut
{
	float finalColor : SV_Target;
};


PSOut main( PSIn input )
{
	PSOut output;
	float shadowBias = 0.0004	// add an extra bias to combat shadow acne
		+ ( input.pos.z - input.viewSpacePos.w / input.pos.w );
	output.finalColor = saturate( length(input.viewSpacePos.xyz) / 100.0f + shadowBias );
	return output;
}

// for MultiPass lighting & transparent objects discard pixel's from the depth shadow map
//	whose alpha (transparency level) is less than your desired threshold.
//	Transparent pixels under that threshold will not cast a shadow, so they won't add their
//		contribution to the shadow map.
//if ( texSample.a < 0.5 )
//	discard;
