////////////////////////////////////////////////////
//		Hyperion Engine	
//		Scene Vertex Shader	
//		© 2021, Zachary Berry
////////////////////////////////////////////////////


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
	matrix WorldMatrix;
}


///////////////////////////
//	Types
///////////////////////////

struct SceneVertex
{
	float4 Position		: POSITION;
	float3 Normal		: NORMAL;
	float2 TexCoords	: TEXCOORD;
};


struct PixelOutput
{
	float4 Position		: SV_POSITION;
	float3 Normal		: NORMAL;
	float3 VSPosition	: POSITIONT;
	float2 TexCoords	: TEXCOORD;
};



///////////////////////////
//	Shader Entry Point
///////////////////////////

PixelOutput main( SceneVertex input )
{
	PixelOutput output;
	input.Position.w = 1.f;

	// Transform input vertex to world space, then view space
	output.Position = mul( input.Position, WorldMatrix );
	output.Position = mul( output.Position, ViewMatrix );

	output.VSPosition = output.Position.xyz;

	// Final transform to screen space
	output.Position = mul( output.Position, ProjectionMatrix );

	// Calculate normal direction in world space
	output.Normal = normalize( mul( input.Normal, (float3x3) WorldMatrix ) );

	// Pass through texture coordinates, to be interpolated
	output.TexCoords = input.TexCoords;

	return output;
}
