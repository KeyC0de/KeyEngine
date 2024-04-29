#include "hlsli/shading_ps.hlsli"
#include "hlsli/point_light_pscb.hlsli"
#include "hlsli/shadowing_ps.hlsli"


cbuffer ModelPCB : register(b0)
{
	bool bSpecularMap;
	bool bSpecularMapAlpha;
	float3 modelSpecularColor;
	float modelSpecularGloss;
};

Texture2D albedoTex : register(t0);
Texture2D specTex : register(t1);
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
		// Specular map contribution
		float modelSpecularGloss_var = modelSpecularGloss;
		float3 specularFactor = float3(0, 0, 0);
		if ( bSpecularMap )
		{
			const float4 specMapColor = specTex.Sample( sampl, input.tc );
			specularFactor = specMapColor.rgb;
			if ( bSpecularMapAlpha )
			{
				modelSpecularGloss_var = pow(2.0f, specMapColor.a * 7.0f );
			}
		}
		else
		{
			specularFactor = modelSpecularColor;
		}
		const float attenuation = calculateLightAttenuation( lv.lengthOfL, attConstant, attLinear, attQuadratic );
		diffuse = calculateLightDiffuseContribution( lightColor, intensity, attenuation, lv.vToL_normalized, input.fragNormalViewSpace );
		specular = calculateLightSpecularContribution( lightColor, specularFactor, intensity, modelSpecularGloss_var, input.fragNormalViewSpace, lv.vToL, input.fragPosViewSpace, attenuation );
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