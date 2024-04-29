#include "hlsli/shading_ps.hlsli"
#include "hlsli/point_light_pscb.hlsli"
#include "hlsli/shadowing_ps.hlsli"


cbuffer ModelPCB : register(b0)
{
	bool bSpecularMap;
	bool bSpecularMapAlpha;
	float3 modelSpecularColor;
	float modelSpecularGloss;
	bool bNormalMap;
	float normalMapStrength;
};

Texture2D albedoTex : register(t0);
Texture2D specTex : register(t1);
Texture2D normTex : register(t2);
SamplerState sampl : register(s0);

struct PSIn
{
	float3 fragPosViewSpace : Position;
	float3 fragNormalViewSpace : Normal;
	float3 tangentViewSpace : Tangent;
	float3 bitangentViewSpace : Bitangent;
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

	PSOut output;
	float4 albedoTexColor = albedoTex.Sample( sampl,
		input.tc );
#ifdef HAS_ALPHA
	clip( albedoTexColor.a < 0.05f ? -1 : 1 );
	// if backfacing fragment flip the normal
	if ( dot( input.fragNormalViewSpace, input.fragPosViewSpace ) >= 0.0f )
	{
		input.fragNormalViewSpace = -input.fragNormalViewSpace;
	}
#endif

	const float shadowLevel = calculateShadowLevel( input.posLightSpace );
	if ( shadowLevel != 0.0f )
	{
		input.fragNormalViewSpace = normalize( input.fragNormalViewSpace );
		if ( bNormalMap )
		{
			const float3 mappedNormal = calculateNormalMapNormal( normalize( input.tangentViewSpace ), normalize( input.bitangentViewSpace ), input.fragNormalViewSpace, input.tc, normTex, sampl );
			input.fragNormalViewSpace = lerp( input.fragNormalViewSpace, mappedNormal, normalMapStrength );
		}
		const PointLightVectors lv = calculatePointLightVectors( pointLightPosViewSpace, input.fragPosViewSpace );
		// Specular map contribution
		float modelSpecularGloss_var = modelSpecularGloss;
		float3 specularFactor = float3(0, 0, 0);
		if ( bSpecularMap )
		{
			const float4 specMapSample = specTex.Sample( sampl, input.tc );
			specularFactor = specMapSample.rgb;
			if ( bSpecularMapAlpha )
			{
				modelSpecularGloss_var = pow(2.0f, specMapSample.a * 7.0f );
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

	output.finalColor = float4(saturate( (diffuse + ambient) * albedoTexColor.rgb + specular ), 1.0f);
	return output;
}
