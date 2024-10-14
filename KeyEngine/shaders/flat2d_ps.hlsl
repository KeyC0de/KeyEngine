Texture2D texture2d : register(t0);
SamplerState sam : register(s0);

struct PSIn
{
	float4 pos : SV_Position;
	float2 uv : TexCoord;
};

struct PSOut
{
	float4 finalColor : SV_Target;
};

PSOut main( PSIn input )
{
	PSOut output;
	output.finalColor = texture2d.Sample( sam, input.uv );
	return output;
}