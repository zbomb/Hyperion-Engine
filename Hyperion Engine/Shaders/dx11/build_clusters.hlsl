////////////////////////////////////////////////////
//		Hyperion Engine	
//		Build Clusters Compute Shader	
//		© 2021, Zachary Berry
////////////////////////////////////////////////////

///////////////////////////
//	Constants
///////////////////////////
#define CLUSTER_EXTRA_SIZE_PERCENT 0.01f;

///////////////////////////
//	Structures
///////////////////////////

struct ClusterInfo
{
	float3 minAABB;
	float _pad_1;
	float3 maxAABB;
	bool bActive;
};

///////////////////////////
//	Constant Buffers
///////////////////////////

cbuffer ScreenBuffer : register( b0 )
{
	float ScreenWidth;
	float ScreenHeight;
	float ScreenNear;
	float ScreenFar;

	uint ShaderMode;
	uint3 _pad_sb_1;

	matrix InvProjectionMatrix;
};

///////////////////////////
//	Resources
///////////////////////////

RWStructuredBuffer< ClusterInfo > clusterList : register( u0 );

//////////////////////////////
//	Function Declarations
//////////////////////////////

float4 screenSpaceToViewSpace( float4 inPoint );
float4 clipSpaceToViewSpace( float4 inPoint );
float3 lineIntersectZPlane( float4 inPoint, float inDepthPlane );

//////////////////////////////
//	Entry Point
//////////////////////////////

[numthreads( 15, 10, 1 )]
void main( uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID )
{
	uint clusterX			= groupThreadID.x;
	uint clusterY			= groupThreadID.y;
	uint clusterZ			= groupID.x;
	uint flatClusterIndex	= clusterX + ( clusterY * 15 ) + ( clusterZ * 150 );
	float clusterSizeX		= ScreenWidth / 15.f;
	float clusterSizeY		= ScreenHeight / 10.f;

	if( ShaderMode == 1 )
	{
		clusterList[ flatClusterIndex ].bActive = false;
	}
	else
	{

		// Calculate AABB points in screen space
		float4 clusterMaxSS = float4( ( clusterX + 1 ) * clusterSizeX, ( clusterY + 1 ) * clusterSizeY, 0.f, 1.f );
		float4 clusterMinSS = float4( clusterX * clusterSizeX, clusterY * clusterSizeY, 0.f, 1.f );

		// Now, we need to take these points into view space
		float4 clusterMaxVS = screenSpaceToViewSpace( clusterMaxSS );
		float4 clusterMinVS = screenSpaceToViewSpace( clusterMinSS );

		// Now, we need to calculate the min and max depth of this cluster
		float clusterNear	= ScreenNear * pow( ScreenFar / ScreenNear, clusterZ / 24.f );
		float clusterFar	= ScreenNear * pow( ScreenFar / ScreenNear, ( clusterZ + 1 ) / 24.f );

		// Next, we need to project a line, from the camera through our points, and intersecting the MinDepth and MaxDepth planes
		// This gives us the final AABB of the cluster
		float3 minNear	= lineIntersectZPlane( clusterMinVS, clusterNear );
		float3 minFar	= lineIntersectZPlane( clusterMinVS, clusterFar );
		float3 maxNear	= lineIntersectZPlane( clusterMaxVS, clusterNear );
		float3 maxFar	= lineIntersectZPlane( clusterMaxVS, clusterFar );

		float3 minAABB	= min( min( minNear, minFar ), min( maxNear, maxFar ) );
		float3 maxAABB	= max( max( minNear, minFar ), max( maxNear, maxFar ) );

		// Ensure the 'deepest' clusters reach all the way to SCREEN_FAR
		if( clusterZ == 23 )
		{
			maxAABB.z = ScreenFar;
		}

		// Store values
		clusterList[ flatClusterIndex ].minAABB = minAABB;
		clusterList[ flatClusterIndex ].maxAABB = maxAABB;
		clusterList[ flatClusterIndex ].bActive = false;
	}
}

//////////////////////////////
//	Helper Functions
//////////////////////////////

float4 clipSpaceToViewSpace( float4 inPoint )
{
	float4 viewPoint = mul( inPoint, InvProjectionMatrix );
	viewPoint /= viewPoint.w;

	return viewPoint;
}


float4 screenSpaceToViewSpace( float4 inPoint )
{
	// Convert from pixels to tex coords
	float2 screenCoords = float2( inPoint.x / ScreenWidth, inPoint.y / ScreenHeight );

	// Convert to clip-space
	float4 clipPoint = float4( screenCoords.x * 2.f - 1.f, ( 1.f - screenCoords.y ) * 2.f - 1.f, inPoint.z, inPoint.w );

	// Convert to view space
	return clipSpaceToViewSpace( clipPoint );
}


float3 lineIntersectZPlane( float4 inPoint, float inDepthPlane )
{
	float3 normal	= float3( 0.f, 0.f, 1.f );
	float3 eye		= float3( 0.f, 0.f, 0.f );

	float3 f = inPoint - eye;
	float t = ( inDepthPlane - dot( normal, eye ) ) / dot( normal, f );

	float3 result = eye + t * f;
	return result;
}
