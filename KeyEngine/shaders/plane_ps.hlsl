#include "hlsli/globals_pscb.hlsli"
#include "hlsli/light_pscb.hlsli"
#include "hlsli/shading_ps.hlsli"
#include "hlsli/shadowing_ps.hlsli"


cbuffer ModelPSCB : register(b0)
{
	float3 modelSpecularColor;
	float modelSpecularGloss;	// the specular power factor
};
Texture2D albedoTex : register(t0);
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
	input.viewSpaceNormal = normalize(input.viewSpaceNormal);

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
				const PointLightVectors plv = calculatePointLightVectors(currentLight.cb_lightPosViewSpace, input.viewSpacePos);

				const float attenuation = calculateLightAttenuation(plv.lengthOfL, currentLight.attConstant, currentLight.attLinear, currentLight.attQuadratic);
				diffuseL = calculateLightDiffuseContribution(currentLight.lightColor, currentLight.intensity, attenuation, plv.vToL_normalized, input.viewSpaceNormal);
				specularL = calculateLightSpecularContribution(currentLight.lightColor, modelSpecularColor, currentLight.intensity, modelSpecularGloss, input.viewSpaceNormal, plv.vToL, input.viewSpacePos, attenuation);
			}

			diffuseL *= shadowLevel;
			specularL *= shadowLevel;
		}

		lightCombinedDiffuse += diffuseL + currentLight.ambient;
		lightCombinedSpecular += specularL;
	}

	float4 albedoTexColor = albedoTex.Sample( sampl, input.tc );
	PSOut output;
	output.finalColor = float4(saturate(lightCombinedDiffuse * albedoTexColor.rgb + lightCombinedSpecular), 1.0f);
	return output;
}