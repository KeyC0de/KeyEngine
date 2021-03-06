
struct VS_to_PS
{
    float4 positionViewport : SV_Position;
    float4 positionClip     : positionClip;
    float2 texCoord         : texCoord;
};


Texture2D NormalsTexture;
Texture2D DepthTexture;
Texture2D RandomTexture;
SamplerState WrapSampler;
SamplerState ClampSampler;

cbuffer MatrixBuffer
{
	matrix cameraProjMatrix;
}

cbuffer SSAOBuffer
{
	float2 screenSize;
	float2 noiseScale; // tiling
	float radius;
	float power;
	float kernelSize;
	float3 kernelOffsets[16];
};

float3 DecodeSphereMap(float2 e)
{
    float2 tmp = e - e * e;
    float f = tmp.x + tmp.y;
    float m = sqrt(4.0f * f - 1.0f);
    
    float3 n;
    n.xy = m * (e * 4.0f - 2.0f);
    n.z  = 3.0f - 8.0f * f;
    return n;
}

float3 ComputePositionViewFromZ(uint2 coords, float zbuffer)
{
	float2 screenPixelOffset = float2(2.0f, -2.0f) / float2(screenSize.x, screenSize.y);
    float2 positionScreen = (float2(coords.xy) + 0.5f) * screenPixelOffset.xy + float2(-1.0f, 1.0f);
	float viewSpaceZ = cameraProjMatrix._43 / (zbuffer - cameraProjMatrix._33);
	
	
    float2 screenSpaceRay = float2(positionScreen.x / cameraProjMatrix._11, positionScreen.y / cameraProjMatrix._22);
    float3 positionView;
    positionView.z = viewSpaceZ;
    positionView.xy = screenSpaceRay.xy * positionView.z;
    
    return positionView;
}

VS_to_PS SSAOVertexShader(uint vertexID : SV_VertexID)
{
    VS_to_PS output;

    output.texCoord = float2((vertexID << 1) & 2, vertexID & 2);
    output.positionClip = float4(output.texCoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    output.positionViewport = output.positionClip;
    
    return output;
}


float4 SSAOPixelShader (VS_to_PS input): SV_Target
{
	float centerZBuffer = DepthTexture.Sample(ClampSampler, input.texCoord).r;
	float3 centerDepthPos = ComputePositionViewFromZ(uint2(input.positionViewport.xy), centerZBuffer);
	float3 normal = DecodeSphereMap(NormalsTexture.Sample(ClampSampler, input.texCoord).xy);
	
	float3 randomVector = RandomTexture.Sample(WrapSampler, input.texCoord * noiseScale).xyz;
	float3 tangent = normalize(randomVector - normal * dot(randomVector, normal));
	float3 bitangent = cross(normal, tangent);
	float3x3 transformMat = float3x3(tangent, bitangent, normal);
	
	float occlusion = 0.0;
	for (int i = 0; i < (int)kernelSize; ++i) 
	{
		float3 samplePos = mul(kernelOffsets[i], transformMat);										
		samplePos = samplePos * radius + centerDepthPos;
					
		float3 sampleDir = normalize(samplePos - centerDepthPos);
 
		float nDotS = max(dot(normal, sampleDir), 0);
					
		float4 offset = mul(float4(samplePos, 1.0), cameraProjMatrix);
		offset.xy /= offset.w;							
																				
		float sampleDepth = DepthTexture.Sample(ClampSampler, float2(offset.x * 0.5 + 0.5, -offset.y * 0.5 + 0.5)).r;
		sampleDepth = ComputePositionViewFromZ(offset.xy, sampleDepth).z;												
		
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(centerDepthPos.z - sampleDepth));
		occlusion += rangeCheck * step(sampleDepth, samplePos.z) * nDotS;
	}
	
	
	occlusion = 1.0 - (occlusion / kernelSize);
	float finalOcclusion = pow(occlusion, power);
	
	return float4(finalOcclusion, finalOcclusion, finalOcclusion, 1.0);
}


