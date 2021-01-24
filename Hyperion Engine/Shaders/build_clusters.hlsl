//
//	Hyperion Engine Shader
//	Cluster Build Compute Shader
//	© 2021, Zachary Berry
//


// Structures
struct ClusterInfo
{
	float4 minAABB;
	float4 maxAABB;
	bool bActive;
	float3 _pad;
};


// Resources
cbuffer ScreenBuffer : register( b0 )
{
	matrix InvProjection;

	uint3 ClusterCount;
	uint _sb_pad_1;
	
	uint2 ClusterScreenSize;
	uint2 ScreenResolution;

	float ScreenNear;
	float ScreenFar;
	float2 _sb_pad_2;
};

RWStructuredBuffer< ClusterInfo > clusterList : register( u0 );

// Function forward declarations
float4 screenSpaceToViewSpace( float4 inPoint );
float4 clipSpaceToViewSpace( float4 inPoint );
float3 lineIntersectZPlane( float4 inPoint, float inDepthPlane );


// Entry Point
// Each thread group will cover a depth slice of the divided frustum
[numthreads(15,10,1)]
void main( uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID )
{
	uint clusterX	= groupThreadID.x;
	uint clusterY	= groupThreadID.y;
	uint clusterZ	= groupID.x;

	// Calculate AABB points in screen space
	float4 clusterMaxSS		= float4( ( clusterX + 1 ) * ClusterScreenSize.x, ( clusterY + 1 ) * ClusterScreenSize.y, 0.f, 1.f );
	float4 clusterMinSS		= float4( clusterX * ClusterScreenSize.x, clusterY * ClusterScreenSize.y, 0.f, 1.f );

	// Now, we need to take these points into view space
	float4 clusterMaxVS		= screenSpaceToViewSpace( clusterMaxSS );
	float4 clusterMinVS		= screenSpaceToViewSpace( clusterMinSS );

	// Now, we need to calculate the min and max depth of this cluster
	float clusterNear	= ScreenNear * pow( ScreenFar / ScreenNear, clusterZ / float( ClusterCount.z ) );
	float clusterFar	= ScreenNear * pow( ScreenFar / ScreenNear, ( clusterZ + 1 ) / float( ClusterCount.z ) );

	// Next, we need to project a line, from the camera through our points, and intersecting the MinDepth and MaxDepth planes
	// This gives us the final AABB of the cluster
	float3 clusterMaxNear	= lineIntersectZPlane( clusterMaxVS, clusterNear );
	float3 clusterMaxFar	= lineIntersectZPlane( clusterMaxVS, clusterFar );
	float3 clusterMinNear	= lineIntersectZPlane( clusterMinVS, clusterNear );
	float3 clusterMinFar	= lineIntersectZPlane( clusterMinVS, clusterFar );

	float3 clusterBoundsMin = min( min( clusterMaxNear, clusterMaxFar ), min( clusterMinNear, clusterMinFar ) );
	float3 clusterBoundsMax = max( max( clusterMaxNear, clusterMaxFar ), min( clusterMinNear, clusterMinFar ) );

	// Finally, store our result in the cluster list
	uint clusterIndex = clusterX + ( clusterY * ClusterCount.x ) + ( clusterZ * ( ClusterCount.x * ClusterCount.y ) );

	clusterList[ clusterIndex ].minAABB = float4( clusterBoundsMin, 1.f );
	clusterList[ clusterIndex ].maxAABB = float4( clusterBoundsMax, 1.f );
	clusterList[ clusterIndex ].bActive = false;
}


float4 clipSpaceToViewSpace( float4 inPoint )
{
	float4 viewPoint = mul( inPoint, InvProjection );
	viewPoint /= viewPoint.w;

	return viewPoint;
}


float4 screenSpaceToViewSpace( float4 inPoint )
{
	// Convert from pixels to tex coords
	float2 screenCoords = float2( inPoint.x / ScreenResolution.x, inPoint.y / ScreenResolution.y );
	
	// Convert to clip-space
	float4 clipPoint = float4( screenCoords.x * 2.f - 1.f, ( 1.f - screenCoords.y ) * 2.f - 1.f, inPoint.z, inPoint.w );

	// Convert to view space
	return clipSpaceToViewSpace( clipPoint );
}


float3 lineIntersectZPlane( float4 inPoint, float inDepthPlane )
{
	float3 normal	= float3( 0.f, 0.f, -1.f );

	float t = inDepthPlane / dot( normal, inPoint.xyz );
	return t * inPoint.xyz;
}