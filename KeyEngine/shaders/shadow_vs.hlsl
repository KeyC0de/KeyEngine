/// \brief	this file is used by the class ShadowPass/DepthPrePass, outputting directly to a Shader Resource View - No Pixel Shader is used
#include "hlsli/transforms_vscb.hlsli"


struct VSIn
{
	float3 pos : Position;
};

struct VSOut
{
	float4 pos : SV_Position;
};

VSOut main( VSIn input )
{
	const float4 inputPos = float4(input.pos, 1.0f);

	VSOut output;
	output.pos = mul( inputPos, cb_worldViewProjection );
	return output;
}