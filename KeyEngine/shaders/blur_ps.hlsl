Texture2D offscreenTex/* : register(t0)*/;
SamplerState sampl/* : register(s0)*/;

static const int radius = 5;
static const float divisor = ( 2 * radius + 1 ) * ( 2 * radius + 1 );

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

	offscreenTex.GetDimensions( width, height );

	// distance {dx,dy} between neighboring pixels
	const float dx = 1.0f / width;
	const float dy = 1.0f / height;

	output.finalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for ( int y = -radius; y <= radius; y++ )
	{
		for ( int x = -radius; x <= radius; x++ )
		{
			const float2 tc = input.uv + float2( dx * x, dy * y );
			output.finalColor += offscreenTex.Sample( sampl, tc ).rgba;
		}
	}

	output.finalColor = output.finalColor / divisor;
	return output;
}