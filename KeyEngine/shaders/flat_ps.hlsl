cbuffer ColorPSCB : register(b0)
{
	float4 materialColor;
};

struct PSOut
{
	float4 finalColor : SV_Target;
};


PSOut main()
{
	PSOut output;
	output.finalColor = float4(materialColor.x, materialColor.y, materialColor.z, 1.0f);
	return output;
}