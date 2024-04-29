cbuffer ColorPCB : register(b0)
{
	float3 materialColor;
};

struct PSOut
{
	float4 finalColor : SV_Target;
};


PSOut main()
{
	PSOut output;
	output.finalColor = float4(materialColor, 1.0f);
	return output;
}