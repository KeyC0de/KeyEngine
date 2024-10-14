#include "hlsli/globals_pscb.hlsli"
#include "hlsli/light_pscb.hlsli"
#include "hlsli/shading_ps.hlsli"
#include "hlsli/shadowing_ps.hlsli"


cbuffer ModelPSCB : register(b0)
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
	float3 viewSpacePos : PositionViewSpace;
	float3 viewSpaceNormal : Normal;
	float2 tc : TexCoord;
	float4 posLightSpace[MAX_LIGHTS] : PositionLightSpace;
};

struct PSOut
{
	float4 finalColor : SV_Target;
};

PSOut main( PSIn input )
{
	input.viewSpaceNormal = normalize( input.viewSpaceNormal );

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
				const PointLightVectors lv = calculatePointLightVectors( currentLight.cb_lightPosViewSpace, input.viewSpacePos );

				const float attenuation = calculateLightAttenuation( lv.lengthOfL, currentLight.attConstant, currentLight.attLinear, currentLight.attQuadratic );
				diffuseL = calculateLightDiffuseContribution( currentLight.lightColor, currentLight.intensity, attenuation, lv.vToL_normalized, input.viewSpaceNormal );
				specularL = calculateLightSpecularContribution( currentLight.lightColor, specularFactor, currentLight.intensity, modelSpecularGloss_var, input.viewSpaceNormal, lv.vToL, input.viewSpacePos, attenuation );
			}

			diffuseL *= shadowLevel;
			specularL *= shadowLevel;
		}

		lightCombinedDiffuse += diffuseL + currentLight.ambient;
		lightCombinedSpecular += specularL;
	}

	PSOut output;
	float4 albedoTexColor = albedoTex.Sample( sampl, input.tc );
	output.finalColor = float4(saturate(lightCombinedDiffuse * albedoTexColor.rgb + lightCombinedSpecular), 1.0f);
	return output;
}