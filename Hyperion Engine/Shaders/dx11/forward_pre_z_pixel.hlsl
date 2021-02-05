////////////////////////////////////////////////////
//		Hyperion Engine	
//		Forward Pre-Z Pixel Shader	
//		© 2021, Zachary Berry
////////////////////////////////////////////////////


///////////////////////////
//	Structures
///////////////////////////

struct PixelInput
{
	float4 Position		: SV_POSITION;
	float3 Normal		: NORMAL;
	float3 VSPosition	: POSITIONT;
	float2 TexCoords	: TEXCOORD;
};

struct ClusterInfo
{
	float3 minAABB;
	float _pad_1;

	float3 maxAABB;
	uint bActive;
};

///////////////////////////
//	Constants
///////////////////////////
cbuffer ViewInfo : register( b0 )
{
	float ScreenWidth;
	float ScreenHeight;
	float DepthSliceA;
	float DepthSliceB;
}

///////////////////////////
//	Resources
///////////////////////////

RWStructuredBuffer< ClusterInfo > clusterList : register( u1 );

///////////////////////////
//	Entry Point
///////////////////////////
void main( PixelInput input )
{
	uint clusterSizeX = uint( ScreenWidth / 15.f );
	uint clusterSizeY = uint( ScreenHeight / 10.f );

	uint clusterX = input.Position.x / clusterSizeX;
	uint clusterY = input.Position.y / clusterSizeY;
	uint clusterZ = floor( log( input.VSPosition.z ) * DepthSliceA - DepthSliceB );

	uint flatIndex = clusterX + ( clusterY * 15 ) + ( clusterZ * 150 );

	InterlockedOr( clusterList[ flatIndex ].bActive, true );
}