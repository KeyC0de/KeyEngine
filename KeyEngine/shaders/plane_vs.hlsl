#include "hlsli/globals_vscb.hlsli"
#include "hlsli/transforms_vscb.hlsli"
#include "hlsli/light_vscb.hlsli"


struct VSIn
{
	float3 pos : Position;
	float3 nrm : Normal;
	float2 tc : TexCoord;
};

struct VSOut
{
	float3 viewSpacePos : PositionViewSpace;
	float3 viewSpaceNormal : Normal;	// only extra VS & PS input required for lighting is Normal
	float2 tc : TexCoord;				// only extra VS & PS input required for texturing is TexCoord
	float4 posLightSpace[MAX_LIGHTS] : PositionLightSpace;	// only extra PS input required for shadowing is PositionLightSpace, so calculate it in VS and pass it to PS
	float4 pos : SV_Position;
};

VSOut main( VSIn input )
{
	// the position vector must be comprised of 4 components for proper matrix calculations
	const float4 inputPos = float4(input.pos, 1.0f);

	VSOut output;
	output.viewSpacePos = (float3) mul( inputPos, cb_worldView );
	output.viewSpaceNormal = mul( input.nrm, (float3x3) cb_worldView );
	output.tc = input.tc;
	output.pos = mul( inputPos, cb_worldViewProjection );

	/*
	HLSL arrays have limited flexibility, especially when dynamically indexed. The compiler sometimes forces unrolling of loops when array indexing is dynamic. This happens because HLSL requires arrays to be fully known and addressable at compile-time (in terms of size and indexing).
	int nNonPointLights = cb_nLights - cb_nPointLights;
	int i = 0;
	[loop]
	for ( ; i < nNonPointLights && i < MAX_LIGHTS; ++i )
	{
		output.posLightSpace[i] = convertVertexPosToNonPointLightSpace( inputPos, cb_world, i );
	}
	[loop]
	for ( ; i < cb_nLights && i < MAX_LIGHTS; ++i )
	{
		output.posLightSpace[i] = convertVertexPosToPointLightSpace( inputPos, cb_world, i );
	}

	Another solution is to use a StructuredBuffer
	StructuredBuffer<float4> posLightSpaceBuffer : register(t0);
	and writing to it instead of an array.
	Structured buffers handle dynamic indexing much better than constant-sized arrays.
	*/
	float4 localPosLightSpace[MAX_LIGHTS];
	int nNonPointLights = cb_nLights - cb_nPointLights;
	int i = 0;
	// #WARNING: Do not attempt to compare arithmetic values of different type in HLSL (eg int with float)
	for ( i = 0; i < cb_nLights && i < MAX_LIGHTS; ++i )
	{
		// firstly iterate through Directional & Spot light
		if (i < nNonPointLights)
		{
			localPosLightSpace[i] = convertVertexPosToNonPointLightSpace(inputPos, cb_world, i);
		}
		// secondly iterate through Point lights
		else
		{
			localPosLightSpace[i] = convertVertexPosToPointLightSpace(inputPos, cb_world, i);
		}
	}
	[unroll]
	for ( i = 0; i < cb_nLights && i < MAX_LIGHTS; ++i )
	{
		output.posLightSpace[i] = localPosLightSpace[i];	// this will force loop to unroll, so I'm adding the [unroll] attribute explicitly to get rid of the warning
	}

	return output;
}