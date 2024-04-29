#include "hlsli/transforms_vscb.hlsli"
#include "hlsli/shadowing_vs.hlsli"


struct VSIn
{
	float3 pos : Position;
	float3 nrm : Normal;
	float2 tc : TexCoord;
};

struct VSOut
{
	float3 viewSpacePos : Position;
	float3 viewSpaceNormal : Normal;
	float2 tc : TexCoord;
	float4 posLightSpace : PositionLightSpace;
	float4 pos : SV_Position;
};


VSOut main( VSIn input )
{
	VSOut output;
	output.viewSpacePos = (float3) mul( float4(input.pos, 1.0f), worldView );
	output.viewSpaceNormal = mul( input.nrm, (float3x3) worldView );
	output.tc = input.tc;
	output.pos = mul( float4(input.pos, 1.0f), worldViewProjection );
	output.posLightSpace = convertVertexPosToLightSpace( input.pos, world );
	return output;
}