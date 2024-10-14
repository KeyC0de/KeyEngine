struct VSIn
{
	float2 pos : Position;
};

struct VSOut
{
	float2 uv : TexCoord;
	float4 pos : SV_Position;
};

VSOut main( VSIn input )
{
	VSOut output;
	output.pos = float4( input.pos, 0.0f, 1.0f );
	output.uv = float2( (input.pos.x + 1) / 2.0f, (1 - input.pos.y) / 2.0f );
	return output;
}