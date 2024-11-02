#define maxRadius 13


Texture2D blurOutlineRtt : register(t0);
SamplerState sampl : register(s0);

cbuffer BlurKernel : register(b0)
{
	uint nTaps;
	float coefficients[maxRadius];
};

cbuffer BlurDirection : register(b1)
{
	bool bHorizontal;
}

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
	uint width;
	uint height;
	float dx;
	float dy;

	blurOutlineRtt.GetDimensions( width, height );
	if ( bHorizontal )
	{
		dx = 1.0f / width;
		dy = 0.0f;
	}
	else
	{
		dx = 0.0f;
		dy = 1.0f / height;
	}
	const int radius = nTaps / 2;
	// output color is max for each RGB component over the entire kernel radius
	//	as I want the outline color to be constant
	float4 maxColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for ( int i = -radius; i <= radius; ++i )
	{
		const float2 tc = input.uv + float2( dx * i, dy * i );
		const float4 col = blurOutlineRtt.Sample( sampl, tc ).rgba;
		const float coef = coefficients[i + radius];
		output.finalColor += col * coef;
		output.finalColor = max( output.finalColor, maxColor );
	}
	return output;
}