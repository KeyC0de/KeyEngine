#include "hlsli/globals_vscb.hlsli"
#include "hlsli/transforms_vscb.hlsli"
#include "hlsli/light_vscb.hlsli"


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
	float3 viewSpacePos : PositionViewSpace;
	float3 viewSpaceNormal : Normal;
	float2 tc : TexCoord;
	float3 tangentViewSpace : Tangent;			// Tangent & Bitangent required for Normal Mapping
	float3 bitangentViewSpace : Bitangent;
	float4 posLightSpace[MAX_LIGHTS] : PositionLightSpace;
	float4 pos : SV_Position;
};

VSOut main( VSIn input )
{
	const float4 inputPos = float4(input.pos, 1.0f);

	VSOut output;
	output.viewSpacePos = (float3) mul( inputPos, cb_worldView );
	output.viewSpaceNormal = mul( input.nrm, (float3x3) cb_worldView );
	output.tc = input.tc;
	output.tangentViewSpace = mul( input.tangent, (float3x3) cb_worldView );
	output.bitangentViewSpace = mul( input.bitangent, (float3x3) cb_worldView );
	output.pos = mul( inputPos, cb_worldViewProjection );

	float4 localPosLightSpace[MAX_LIGHTS];
	int nNonPointLights = cb_nLights - cb_nPointLights;
	int i = 0;
	for ( i = 0; i < cb_nLights && i < MAX_LIGHTS; ++i )
	{
		if (i < nNonPointLights)
		{
			localPosLightSpace[i] = convertVertexPosToNonPointLightSpace(inputPos, cb_world, i);
		}
		else
		{
			localPosLightSpace[i] = convertVertexPosToPointLightSpace(inputPos, cb_world, i);
		}
	}
	[unroll]
	for ( i = 0; i < cb_nLights && i < MAX_LIGHTS; ++i )
	{
		output.posLightSpace[i] = localPosLightSpace[i];
	}

	return output;
}