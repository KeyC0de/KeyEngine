#include "hlsli/globals_pscb.hlsli"
#include "hlsli/light_pscb.hlsli"
#include "hlsli/shading_ps.hlsli"
#include "hlsli/shadowing_ps.hlsli"


cbuffer ModelPSCB : register(b0)
{
	bool cb_bSpecularMap;
	bool cb_bSpecularMapAlpha;
	float3 cb_modelSpecularColor;
	float cb_modelSpecularGloss;
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

	float cb_modelSpecularGloss_var = cb_modelSpecularGloss;
	float3 specularFactor = float3(0, 0, 0);
	if ( cb_bSpecularMap )
	{
		const float4 specMapColor = specTex.Sample( sampl, input.tc );
		specularFactor = specMapColor.rgb;
		if ( cb_bSpecularMapAlpha )
		{
			cb_modelSpecularGloss_var = pow(2.0f, specMapColor.a * 7.0f );
		}
	}
	else
	{
		specularFactor = cb_modelSpecularColor;
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
				const LightVectors lv = calculatePointLightVectors( currentLight.cb_lightPosViewSpace, input.viewSpacePos );

				const float attenuation = calculateLightAttenuation( lv.lengthOfL, currentLight.cb_attConstant, currentLight.cb_attLinear, currentLight.cb_attQuadratic );
				diffuseL = calculateLightDiffuseContribution( currentLight.cb_lightColor, currentLight.cb_intensity, attenuation, lv.LNormalized, input.viewSpaceNormal );
				specularL = calculateLightSpecularContribution( currentLight.cb_lightColor, specularFactor, currentLight.cb_intensity, cb_modelSpecularGloss_var, input.viewSpaceNormal, lv.L, input.viewSpacePos, attenuation );
			}

			diffuseL *= shadowLevel;
			specularL *= shadowLevel;
		}

		lightCombinedDiffuse += diffuseL;
		lightCombinedSpecular += specularL;
	}

	float4 albedoTexColor = albedoTex.Sample( sampl, input.tc );
	PSOut output;
	output.finalColor = float4(saturate(lightCombinedDiffuse * albedoTexColor.rgb + lightCombinedSpecular + g_ambientColor), 1.0f);
	return output;
}