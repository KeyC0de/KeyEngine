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
	float3 diffuseL;
	float3 specularL;
	const float shadowLevel = calculateShadowLevel( input.posLightSpace );
	if ( shadowLevel != 0.0f )
	{
		input.viewSpaceNormal = normalize( input.viewSpaceNormal );
		const PointLightVectors plv = calculatePointLightVectors( pointLightPosViewSpace, input.viewSpacePos );
		const float attenuation = calculateLightAttenuation( plv.lengthOfL, attConstant, attLinear, attQuadratic );
		diffuseL = calculateLightDiffuseContribution( lightColor, intensity, attenuation, plv.vToL_normalized, input.viewSpaceNormal );
		specularL = calculateLightSpecularContribution( lightColor, modelSpecularColor, intensity, modelSpecularGloss, input.viewSpaceNormal, plv.vToL, input.viewSpacePos, attenuation );
		diffuseL *= shadowLevel;
		specularL *= shadowLevel;
	}
	else
	{
		diffuseL = specularL = 0.0f;
	}

	PSOut output;
	float4 albedoTexColor = albedoTex.Sample( sampl, input.tc );
	output.finalColor = float4( saturate( (diffuseL + ambient) * albedoTexColor.rgb + specularL ), 1.0f );
	//or output.finalColor = float4( saturate( (diffuseL + ambient) * albedoTexColor.rgb + specularL ), albedoTexColor.a );
	return output;
}