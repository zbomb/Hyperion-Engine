////////////////////////////////////////////////////
//		Hyperion Engine	
//		Screen Vertex Shader	
//		© 2021, Zachary Berry
////////////////////////////////////////////////////


///////////////////////////
//	Constant Buffers
///////////////////////////

cbuffer MatrixBuffer : register( b0 )
{
	matrix worldViewProjMatrix;
}

///////////////////////////
//	Structures
///////////////////////////

struct VertexInput
{
	float4 Position		: POSITION;
	float2 TexCoords	: TEXCOORD;
};


struct VertexOutput
{
	float4 Position		: SV_POSITION;
	float2 TexCoords	: TEXCOORD;
};

///////////////////////////
//	Entry Point
///////////////////////////

VertexOutput main( VertexInput input )
{
	VertexOutput output;
	input.Position.w = 1.f;

	// Calculate transformed vertex position, depth should always be zero
	output.Position		= mul( input.Position, worldViewProjMatrix );
	output.Position.z	= 0.f;

	// Pass through texture coords to be interpolated
	output.TexCoords = input.TexCoords;

	return output;
}