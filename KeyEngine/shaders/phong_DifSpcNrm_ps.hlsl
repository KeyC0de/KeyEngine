#include "hlsli/globals_pscb.hlsli"
#include "hlsli/light_pscb.hlsli"
#include "hlsli/shading_ps.hlsli"
#include "hlsli/shadowing_ps.hlsli"


/// \brief PS Resources section start

/// \brief order of elements must be observed between the ModelPSCB and the order of elements added in `con::RawLayout cbLayout;` in `MaterialLoader::MaterialLoader`
cbuffer ModelPSCB : register(b0)
{
	bool cb_bSpecularMap;
	bool cb_bSpecularMapAlpha;
	float3 cb_modelSpecularColor;
	float cb_modelSpecularGloss;	// the specular power factor
	bool cb_bNormalMap;
	float cb_normalMapStrength;
};

Texture2D albedoTex : register(t0);
Texture2D specTex : register(t1);
Texture2D normTex : register(t2);
SamplerState sampl : register(s0);
/// \brief PS Resources section end

/// \brief PS Input
struct PSIn
{
	float3 viewSpacePos : PositionViewSpace;
	float3 viewSpaceNormal : Normal;	// only extra PS input required for lighting calcs is Normal
	float2 tc : TexCoord;				// only extra PS input required for texturing calcs is TexCoord
	float3 tangentViewSpace : Tangent;
	float3 bitangentViewSpace : Bitangent;
	float4 posLightSpace[MAX_LIGHTS] : PositionLightSpace;	// only extra PS input required for shadowing calcs is PositionLightSpace, we calculate it in VS and pass it to PS (or even better #OPTIMIZATION: calculate it in PS directly)
};

/// \brief PS Output
struct PSOut
{
	float4 finalColor : SV_Target;
};

/// \brief PS entry point
PSOut main( PSIn input )
{
	float4 modelDiffuseColor = albedoTex.Sample( sampl, input.tc );
#ifdef HAS_ALPHA
	clip( modelDiffuseColor.a < 0.05f ? -1 : 1 );	// `clip` calls `discard` internally; this isn't required if Blending is enabled
	// if backfacing fragment flip the normal to look at the camera
	if ( dot( input.viewSpaceNormal, input.viewSpacePos ) >= 0.0f )
	{
		input.viewSpaceNormal = -input.viewSpaceNormal;
	}
#endif

	// viewSpaceNormal is the mesh normal. It will come out in the PS interpolated; so we need to normalize it
	input.viewSpaceNormal = normalize(input.viewSpaceNormal);

	// get normal map contribution
	if (cb_bNormalMap)
	{
		const float3 mappedNormalViewSpace = calculateNormalMapNormal(normalize(input.tangentViewSpace), normalize(input.bitangentViewSpace), input.viewSpaceNormal, input.tc, normTex, sampl);
		// lerp mesh normal with the normal map's normal
		input.viewSpaceNormal = lerp( input.viewSpaceNormal, mappedNormalViewSpace, cb_normalMapStrength );
	}

	// Specular map contribution
	float modelSpecularGloss = cb_modelSpecularGloss;
	float3 modelSpecularColor = float3(0, 0, 0);
	if (cb_bSpecularMap)
	{
		const float4 specularTexColor = specTex.Sample( sampl, input.tc );
		modelSpecularColor = specularTexColor.rgb;
		if (cb_bSpecularMapAlpha)
		{
			modelSpecularGloss = pow( 2.0f, specularTexColor.a * 7.0f );
		}
	}
	else
	{
		modelSpecularColor = cb_modelSpecularColor;
	}

	// if another pixel is occluding this one, then this one will be in shadow, so don't apply lighting and only add an cb_ambientColor light term
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
				shadowLevel = calculateShadowLevelCubeMapArray(input.posLightSpace[i], i, currentLight.cb_shadowCamNearZ, currentLight.cb_shadowCamFarZ);
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
				const LightVectors lv = calculatePointLightVectors(currentLight.cb_lightPosViewSpace, input.viewSpacePos);

				const float attenuation = calculateLightAttenuation(lv.lengthOfL, currentLight.cb_attConstant, currentLight.cb_attLinear, currentLight.cb_attQuadratic);
				diffuseL = calculateLightDiffuseContribution(currentLight.cb_lightColor, currentLight.cb_intensity, attenuation, lv.LNormalized, input.viewSpaceNormal);
				specularL = calculateLightSpecularContribution(currentLight.cb_lightColor, modelSpecularColor, currentLight.cb_intensity, modelSpecularGloss, input.viewSpaceNormal, lv.L, input.viewSpacePos, attenuation);
			}

			diffuseL *= shadowLevel;
			specularL *= shadowLevel;
		}

		lightCombinedDiffuse += diffuseL;
		lightCombinedSpecular += specularL;
	}

	PSOut output;
	output.finalColor = float4(saturate(lightCombinedDiffuse * modelDiffuseColor.rgb + lightCombinedSpecular /** modelSpecularColor*/ + g_ambientColor), 1.0f);	// #TODO: output alpha properly at some point
	return output;
}