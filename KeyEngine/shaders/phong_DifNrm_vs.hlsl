#include "hlsli/transforms_vscb.hlsli"
#include "hlsli/shadowing_vs.hlsli"


struct VSIn
{
	float3 pos : Position;
	float3 nrm : Normal;
	float2 tc : TexCoord;
	float3 tangent : Tangent;
	float3 bitangent : Bitangent;
};

struct VSOut
{
	float3 viewSpacePos : Position;
	float3 normalViewSpace : Normal;
	float3 tangent : Tangent;
	float3 bitangent : Bitangent;
	float2 tc : TexCoord;
	float4 posLightSpace : PositionLightSpace;
	float4 pos : SV_Position;
};


VSOut main( VSIn input )
{
	VSOut output;
	output.viewSpacePos = (float3) mul( float4(input.pos, 1.0f), worldView );
	output.normalViewSpace = mul( input.nrm, (float3x3) worldView );
	output.tangent = mul( input.tangent, (float3x3) worldView );
	output.bitangent = mul( input.bitangent, (float3x3) worldView );
	output.pos = mul( float4(input.pos, 1.0f), worldViewProjection );
	output.tc = input.tc;
	output.posLightSpace = convertVertexPosToLightSpace( input.pos, world );
	return output;
}