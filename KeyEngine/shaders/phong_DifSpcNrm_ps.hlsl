#include "hlsli/globals_pscb.hlsli"
#include "hlsli/light_pscb.hlsli"
#include "hlsli/shading_ps.hlsli"
#include "hlsli/shadowing_ps.hlsli"


// PS Resources
cbuffer ModelPSCB : register(b0)
{
	bool bSpecularMap;
	bool bSpecularMapAlpha;
	float3 modelSpecularColor;
	float modelSpecularGloss;	// the specular power factor
	bool bNormalMap;
	float normalMapStrength;
};

Texture2D albedoTex : register(t0);
Texture2D specTex : register(t1);
Texture2D normTex : register(t2);
SamplerState sampl : register(s0);

// PS Input
struct PSIn
{
	float3 viewSpacePos : PositionViewSpace;
	float3 viewSpaceNormal : Normal;
	float2 tc : TexCoord;
	float3 tangentViewSpace : Tangent;
	float3 bitangentViewSpace : Bitangent;
	float4 posLightSpace[MAX_LIGHTS] : PositionLightSpace;
};

// PS Output
struct PSOut
{
	float4 finalColor : SV_Target;
};

PSOut main( PSIn input )
{
	float4 albedoTexColor = albedoTex.Sample(sampl, input.tc);
#ifdef HAS_ALPHA
	clip( albedoTexColor.a < 0.05f ? -1 : 1 );
	// if backfacing fragment flip the normal to look at the camera
	if ( dot( input.viewSpaceNormal, input.viewSpacePos ) >= 0.0f )
	{
		input.viewSpaceNormal = -input.viewSpaceNormal;
	}
#endif

	// normalize the mesh normal
	input.viewSpaceNormal = normalize(input.viewSpaceNormal);

	// get normal map contribution
	if (bNormalMap)
	{
		const float3 mappedNormal = calculateNormalMapNormal(normalize(input.tangentViewSpace), normalize(input.bitangentViewSpace), input.viewSpaceNormal, input.tc, normTex, sampl);
		// lerp normal with the normal map's normal if there's a normal map
		input.viewSpaceNormal = lerp(input.viewSpaceNormal, mappedNormal, normalMapStrength);
	}

	// Specular map contribution
	float modelSpecularGloss_var = modelSpecularGloss;
	float3 specularFactor = float3(0, 0, 0);
	if (bSpecularMap)
	{
		const float4 specularTexColor = specTex.Sample(sampl, input.tc);
		specularFactor = specularTexColor.rgb;
		if (bSpecularMapAlpha)
		{
			modelSpecularGloss_var = pow(2.0f, specularTexColor.a * 7.0f);
		}
	}
	else
	{
		specularFactor = modelSpecularColor;
	}

	// if another pixel is occluding this one, then this one will be in shadow
	// so don't apply lighting and only add an ambient light term
	float3 lightCombinedDiffuse = float3(0, 0, 0);
	float3 lightCombinedSpecular = float3(0, 0, 0);

	for ( int i = 0; i < cb_nLights; ++i )
	{
		float3 diffuseL = 0.0f;
		float3 specularL = 0.0f;
		float shadowLevel = 0.0f;
		LightProperties currentLight = cb_lights[i];

		if ( currentLight.cb_bCastingShadows )
		{
			if (currentLight.cb_lightType == 1 || currentLight.cb_lightType == 2)
			{
				//shadowLevel = calculateShadowLevelMapArray(input.posLightSpace[i], i);	// #TODO:
			}
			else if (currentLight.cb_lightType == 3)
			{
				shadowLevel = calculateShadowLevelCubeMapArray(input.posLightSpace[i], i);
			}
		}

		if ( shadowLevel != 0.0f )
		{
			if (currentLight.cb_lightType == 1)
			{
				// #TODO:
			}
			else if (currentLight.cb_lightType == 2)
			{
				// #TODO:
			}
			else if (currentLight.cb_lightType == 3)
			{
				const PointLightVectors lv = calculatePointLightVectors(currentLight.cb_lightPosViewSpace, input.viewSpacePos);

				const float attenuation = calculateLightAttenuation(lv.lengthOfL, currentLight.attConstant, currentLight.attLinear, currentLight.attQuadratic);
				diffuseL = calculateLightDiffuseContribution(currentLight.lightColor, currentLight.intensity, attenuation, lv.vToL_normalized, input.viewSpaceNormal);
				specularL = calculateLightSpecularContribution(currentLight.lightColor, specularFactor, currentLight.intensity, modelSpecularGloss_var, input.viewSpaceNormal, lv.vToL, input.viewSpacePos, attenuation);
			}

			diffuseL *= shadowLevel;
			specularL *= shadowLevel;
		}

		lightCombinedDiffuse += diffuseL + currentLight.ambient;
		lightCombinedSpecular += specularL;
	}

	PSOut output;
	output.finalColor = float4(saturate(lightCombinedDiffuse * albedoTexColor.rgb + lightCombinedSpecular), 1.0f);	// #TODO: output alpha properly at some point
	return output;
}