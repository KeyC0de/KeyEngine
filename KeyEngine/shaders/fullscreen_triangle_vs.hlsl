// currently unused
struct VSIn
{
	float2 pos : Position;
	uint id : SV_VertexID;
};

struct VSOut
{
	float2 uv : TexCoord;
	float4 pos : SV_Position;
};

VSOut main( VSIn input )
{
	VSOut output;
	output.uv = float2((input.id << 1) & 2, input.id & 2);
	output.pos = float4(output.uv * float2(2, -2) + float2(-1, 1), 1, 1);
	return output;
}
// Use Draw(3,0) to draw; only VB & IB are needed