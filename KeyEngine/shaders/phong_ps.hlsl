#include "hlsli/shading.hlsli"
#include "hlsli/point_light_pcb.hlsli"
#include "hlsli/shadowing_ps.hlsli"


cbuffer ModelPCB : register(b0)
{
	float3 materialColor;
	float3 modelSpecularColor;
	float modelSpecularGloss;
};

struct PSIn
{
	float3 fragPosViewSpace : Position;
	float3 fragNormalViewSpace : Normal;
	float4 posLightSpace : PositionLightSpace;
};

struct PSOut
{
	float4 finalColor : SV_Target;
};


PSOut main( PSIn input )
{
	PSOut output;
	float3 diffuse;
	float3 specular;
	
	const float shadowLevel = calculateShadowLevel( input.posLightSpace );
	if ( shadowLevel != 0.0f )
	{
		input.fragNormalViewSpace = normalize( input.fragNormalViewSpace );
		const PointLightVectors lv = calculatePointLightVectors( pointLightPosViewSpace,
			input.fragPosViewSpace );
		const float att = calculateLightAttenuation( lv.lengthOfL,
			attConstant,
			attLinear,
			attQuadratic );
		diffuse = calculateLightDiffuseContribution( lightColor,
			intensity,
			att,
			lv.vToL_normalized,
			input.fragNormalViewSpace );
		specular = calculateLightSpecularContribution( lightColor,
			modelSpecularColor,
			intensity,
			modelSpecularGloss,
			input.fragNormalViewSpace,
			lv.vToL,
			input.fragPosViewSpace,
			att );
		diffuse *= shadowLevel;
		specular *= shadowLevel;
	}
	else
	{
		diffuse = specular = 0.0f;
	}
	
	output.finalColor = float4(saturate( (diffuse + ambient) * materialColor + specular ),
		1.0f );
	return output;
}