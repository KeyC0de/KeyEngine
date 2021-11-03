Texture2D shaderTexture;
SamplerState SampleType;

// The PixelInputType has the added color component in the pixel shader also.

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR;
};


float4 main(PixelInputType input) : SV_TARGET
{
	float4 textureColor;
	float4 finalColor;

	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColor = shaderTexture.Sample(SampleType, input.tex);
	// Here is where we combine the texture color and the input particle color to get the final output color.

	// Combine the texture color and the particle color to get the final color result.
	finalColor = textureColor * input.color;

	return finalColor;
}