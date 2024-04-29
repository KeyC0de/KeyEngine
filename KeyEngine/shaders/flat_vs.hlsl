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
	VSOut output;
	output.pos = mul( float4(input.pos, 1.0f), worldViewProjection );
	return output;
}