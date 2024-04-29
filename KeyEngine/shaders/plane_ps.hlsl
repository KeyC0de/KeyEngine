#include "hlsli/shading_ps.hlsli"
#include "hlsli/shadowing_ps.hlsli"
#include "hlsli/point_light_pscb.hlsli"


cbuffer ModelPCB : register(b0)
{
	float3 modelSpecularColor;
	float modelSpecularGloss;	// the specular power factor
};
Texture2D albedoTex : register(t0);
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
		const PointLightVectors plv = calculatePointLightVectors( pointLightPosViewSpace, input.viewSpacePos );
		const float attenuation = calculateLightAttenuation( plv.lengthOfL, attConstant, attLinear, attQuadratic );
		diffuse = calculateLightDiffuseContribution( lightColor, intensity, attenuation, plv.vToL_normalized, input.viewSpaceNormal );
		specular = calculateLightSpecularContribution( lightColor, modelSpecularColor, intensity, modelSpecularGloss, input.viewSpaceNormal, plv.vToL, input.viewSpacePos, attenuation );
		diffuse *= shadowLevel;
		specular *= shadowLevel;
	}
	else
	{
		diffuse = specular = 0.0f;
	}

	PSOut output;
	float4 albedoTexColor = albedoTex.Sample( sampl, input.tc );
	output.finalColor = float4(saturate( (diffuse + ambient) * albedoTexColor.rgb + specular ), 1.0f );
	return output;
}