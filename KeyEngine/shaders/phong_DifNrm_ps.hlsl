#include "hlsli/shading_ps.hlsli"
#include "hlsli/point_light_pscb.hlsli"
#include "hlsli/shadowing_ps.hlsli"


cbuffer ModelPCB : register(b0)
{
	float3 modelSpecularColor;
	float modelSpecularGloss;
	bool bNormalMap;
	float normalMapStrength;
};

Texture2D albedoTex : register(t0);
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

	// shadow occlusion test
	const float shadowLevel = calculateShadowLevel( input.posLightSpace );
	if ( shadowLevel != 0.0f )
	{
		// normalize the mesh normal
		input.fragNormalViewSpace = normalize( input.fragNormalViewSpace );
		// lerp normal with the normal map's normal if there's a normal map
		if ( bNormalMap )
		{
			const float3 mappedNormal = calculateNormalMapNormal( normalize( input.tangentViewSpace ), normalize( input.bitangentViewSpace ), input.fragNormalViewSpace, input.tc, normTex, sampl );
			input.fragNormalViewSpace = lerp( input.fragNormalViewSpace, mappedNormal, normalMapStrength );
		}
		const PointLightVectors lv = calculatePointLightVectors( pointLightPosViewSpace, input.fragPosViewSpace );
		// light attenuation
		const float attenuation = calculateLightAttenuation( lv.lengthOfL, attConstant, attLinear, attQuadratic );
		// diffuse contribution
		diffuse = calculateLightDiffuseContribution( lightColor, intensity, attenuation, lv.vToL_normalized, input.fragNormalViewSpace );
		// specular contribution
		specular = calculateLightSpecularContribution( lightColor, modelSpecularColor, intensity, modelSpecularGloss, input.fragNormalViewSpace, lv.vToL, input.fragPosViewSpace, attenuation );
		// scale contributions by shadow level
		diffuse *= shadowLevel;
		specular *= shadowLevel;
	}
	else
	{
		// if another pixel is occluding this one, then this one will be in shadow
		// so don't apply lighting and only add an ambient light term
		diffuse = specular = 0.0f;
	}

	// #TODO: check HAS_ALPHA and work the alpha output component
	// calculate texture contribution
	PSOut output;
	float4 albedoTexColor = albedoTex.Sample( sampl, input.tc );
	output.finalColor = float4(saturate( (diffuse + ambient) * albedoTexColor.rgb + specular ), 1.0f);
	return output;
}