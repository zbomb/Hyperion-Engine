//
//	Hyperion Engine Shader
//	GBuffer Vertex Shader
//	© 2021, Zachary Berry
//

// Constant Buffers
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


// Types
struct VertexInput
{
	float4 Position : POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD0;
};

struct PixelOutput
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD0;
};


// Vertex Shader Program
PixelOutput main( VertexInput inVertex )
{
	PixelOutput output;
	inVertex.Position.w = 1.f;

	// Calculate screenspace position of the vertex based on the matricies
	output.Position = mul( inVertex.Position, worldMatrix );
	output.Position = mul( output.Position, viewMatrix );
	output.Position = mul( output.Position, projectionMatrix );

	// Pass through the texture coords
	output.TexCoord = inVertex.TexCoord;

	// Calculate surface normal
	output.Normal = normalize( mul( inVertex.Normal, ( float3x3 ) worldMatrix ) );

	return output;
}