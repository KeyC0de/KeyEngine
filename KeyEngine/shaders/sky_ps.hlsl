TextureCube albedoTex : register(t0);
SamplerState sampl : register(s0);

struct PSIn
{
	float3 worldPos : Position;
};

struct PSOut
{
	float4 finalColor : SV_Target;
};


PSOut main( PSIn input )
{
	PSOut output;
	output.finalColor = albedoTex.Sample( sampl, input.worldPos );
	return output;
}