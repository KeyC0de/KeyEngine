#include "hlsli/shading.hlsli"
#include "hlsli/shadowing_ps.hlsli"
#include "hlsli/point_light_pcb.hlsli"


cbuffer ModelPCB : register(b0)
{
	float3 modelSpecularColor;
	float modelSpecularGloss;	// the specular power factor
};
Texture2D diffTex : register(t0);
SamplerState sampl : register(s0);

struct PSIn
{
	float3 viewSpacePos : Position;
	float3 viewSpaceNormal : Normal;
	float2 tc : TexCoord;
	float4 posLightSpace : PositionLightSpace;
};

struct PSOut
{
	float4 finalColor : SV_Target;
};


PSOut main( PSIn input )
{
	float3 diffuse;
	float3 specular;
	const float shadowLevel = calculateShadowLevel( input.posLightSpace );
	if ( shadowLevel != 0.0f )
	{
		input.viewSpaceNormal = normalize( input.viewSpaceNormal );
		const PointLightVectors plv = calculatePointLightVectors( pointLightPosViewSpace,
			input.viewSpacePos );
		const float att = calculateLightAttenuation( plv.lengthOfL,
			attConstant,
			attLinear,
			attQuadratic );
		diffuse = calculateLightDiffuseContribution( lightColor,
			intensity,
			att,
			plv.vToL_normalized,
			input.viewSpaceNormal );
		specular = calculateLightSpecularContribution( lightColor,
			modelSpecularColor,
			intensity,
			modelSpecularGloss,
			input.viewSpaceNormal,
			plv.vToL,
			input.viewSpacePos,
			att );
		diffuse *= shadowLevel;
		specular *= shadowLevel;
	}
	else
	{
		diffuse = specular = 0.0f;
	}
	
	PSOut output;
	float4 diffTexColor = diffTex.Sample( sampl,
		input.tc );
	output.finalColor = float4(saturate( (diffuse + ambient) * diffTexColor.rgb + specular ),
		1.0f );
	return output;
}