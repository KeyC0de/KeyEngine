cbuffer SkyboxTransformVSCB : register(b0)
{
	matrix viewProjection;
};

struct VSIn
{
	float3 worldPos : Position;
};

struct VSOut
{
	float3 worldPos : PositionWorldSpace;
	float4 pos : SV_Position;
};

VSOut main( VSIn input )
{
	VSOut output;
	output.worldPos = input.worldPos;
	output.pos = mul( float4(input.worldPos, 0.0f), viewProjection );
	// make sure that z after perspective (w) divide will be 1.0; such that our special DepthStencilState mode "DepthEquals1" will do its magic
	output.pos.z = output.pos.w;
	return output;
}