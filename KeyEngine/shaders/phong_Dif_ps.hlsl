#include "hlsli/shading_ps.hlsli"
#include "hlsli/point_light_pscb.hlsli"
#include "hlsli/shadowing_ps.hlsli"


cbuffer ModelPCB : register(b0)
{
	float3 modelSpecularColor;
	float modelSpecularGloss;
};
Texture2D albedoTex : register(t0);
SamplerState sampl : register(s0);

struct PSIn
{
	float3 fragPosViewSpace : Position;
	float3 fragNormalViewSpace : Normal;
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
		input.fragNormalViewSpace = normalize( input.fragNormalViewSpace );
		const PointLightVectors lv = calculatePointLightVectors( pointLightPosViewSpace, input.fragPosViewSpace );
		const float attenuation = calculateLightAttenuation( lv.lengthOfL, attConstant, attLinear, attQuadratic );
		diffuse = calculateLightDiffuseContribution( lightColor, intensity, attenuation, lv.vToL_normalized, input.fragNormalViewSpace );
		specular = calculateLightSpecularContribution( lightColor, modelSpecularColor, intensity, modelSpecularGloss, input.fragNormalViewSpace, lv.vToL, input.fragPosViewSpace, attenuation );
		diffuse *= shadowLevel;
		specular *= shadowLevel;
	}
	else
	{
		diffuse = specular = 0.0f;
	}

	PSOut output;
	float4 albedoTexColor = albedoTex.Sample( sampl, input.tc );
	output.finalColor = float4(saturate( (diffuse + ambient) * albedoTexColor.rgb + specular ), 1.0f);
	return output;
}