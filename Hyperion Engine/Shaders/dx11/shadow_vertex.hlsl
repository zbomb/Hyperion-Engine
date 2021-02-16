////////////////////////////////////////////////////
//		Hyperion Engine	
//		Shadow Vertex Shader	
//		© 2021, Zachary Berry
////////////////////////////////////////////////////

#define MAX_INSTANCE_COUNT 512

///////////////////////////
//	Constant Buffers
///////////////////////////

cbuffer StaticMatrixBuffer : register( b0 )
{
	matrix ViewMatrix;
	matrix ProjectionMatrix;
}


cbuffer ObjectMatrixBuffer : register( b1 )
{
	matrix WorldMatrix[ MAX_INSTANCE_COUNT ];
}


///////////////////////////
//	Types
///////////////////////////

struct SceneVertex
{
	float4 Position		: POSITION;
	float3 Normal		: NORMAL;
	float2 TexCoords	: TEXCOORD;
	uint BatchInstance	: SV_InstanceID;
};


struct PixelOutput
{
	float4 Position	: SV_Position;
};



///////////////////////////
//	Shader Entry Point
///////////////////////////

PixelOutput main( SceneVertex input )
{
	PixelOutput output;
	input.Position.w = 1.f;

	// Transform input vertex to world space, then view space
	output.Position = mul( input.Position, WorldMatrix[ input.BatchInstance ] );
	output.Position = mul( output.Position, ViewMatrix );
	output.Position = mul( output.Position, ProjectionMatrix );

	return output;
}
