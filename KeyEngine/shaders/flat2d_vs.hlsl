struct VSIn
{
	float4 pos : Position;
	float2 uv : TexCoord;
};

struct VSOut
{
	float4 pos : SV_Position;
	float2 uv : TexCoord;
};

VSOut main( VSIn input )
{
	VSOut output;
	output.pos = input.pos;
	output.uv = input.uv;
	return output;
}