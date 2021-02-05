////////////////////////////////////////////////////
//		Hyperion Engine	
//		Find Clusters Compute Shader	
//		© 2021, Zachary Berry
////////////////////////////////////////////////////


///////////////////////////
//	Structures
///////////////////////////

struct ClusterInfo
{
	float3 minAABB;
	float _pad_1;
	float3 maxAABB;
	uint bActive; // Interpret this as a uint, instead of bool, so we can use interlocked or
};

///////////////////////////
//	Constant Buffers
///////////////////////////

cbuffer screenInfo : register( b0 )
{
	float ScreenWidth;
	float ScreenHeight;
	float DepthSliceA;
	float DepthSliceB;

	float ScreenNear;
	float ScreenFar;
	float2 _pad_si_1;
};

///////////////////////////
//	Resources
///////////////////////////

RWStructuredBuffer< ClusterInfo > clusterList	: register( u0 );
Texture2D depthBuffer							: register( t0 );
SamplerState depthSampler						: register( s0 );

//////////////////////////////
//	Entry Point
//////////////////////////////

[numthreads( 1, 1, 1 )]
void main( uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID )
{
	uint pixelX = groupID.x;
	uint pixelY = groupID.y;

	uint clusterSizeX = uint( ScreenWidth / 15.f );
	uint clusterSizeY = uint( ScreenHeight / 10.f );

	uint clusterX = pixelX / clusterSizeX;
	uint clusterY = pixelY / clusterSizeY;

	float texelX = float( pixelX + 0.5f ) / ScreenWidth;
	float texelY = float( pixelY - 0.5f ) / ScreenHeight;

	float pixelDepth	= depthBuffer.SampleLevel( depthSampler, float2( texelX, texelY ), 0.f ).w;
	uint clusterZ		= floor( log( pixelDepth ) * DepthSliceA - DepthSliceB );
	uint flatIndex		= clusterX + ( clusterY * 15 ) + ( clusterZ * 150 );

	InterlockedOr( clusterList[ flatIndex ].bActive, true );
}