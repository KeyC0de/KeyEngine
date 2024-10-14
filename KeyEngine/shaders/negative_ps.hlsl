Texture2D offscreenTex/* : register(t0)*/;
SamplerState sampl/* : register(s0)*/;

struct PSIn
{
	float2 uv : TexCoord;
};

struct PSOut
{
	float4 finalColor : SV_Target;
};

PSOut main( PSIn input )
{
	PSOut output;
	output.finalColor = 1.0f - offscreenTex.Sample( sampl, input.uv ).rgba;
	return output;
}
