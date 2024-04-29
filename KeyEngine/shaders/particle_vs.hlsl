cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


// The two input types both have a color component so that the particle can have an individual color that is added to the texture base color.

struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR;
};


PixelInputType main( VertexInputType input )
{
	PixelInputType output;


	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// #TODO: orient them to face the camera

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	// Store the particle color for the pixel shader.
	output.color = input.color;

	return output;
}