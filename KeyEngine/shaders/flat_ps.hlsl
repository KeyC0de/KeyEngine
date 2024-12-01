cbuffer ColorPSCB : register(b0)
{
	float4 cb_materialColor;
};

struct PSOut
{
	float4 finalColor : SV_Target;
};

PSOut main()
{
	PSOut output;
	output.finalColor = float4(cb_materialColor.x, cb_materialColor.y, cb_materialColor.z, 1.0f);
	return output;
}