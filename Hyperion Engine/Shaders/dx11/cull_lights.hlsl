////////////////////////////////////////////////////
//		Hyperion Engine	
//		Cull Lights Compute Shader	
//		© 2021, Zachary Berry
////////////////////////////////////////////////////

///////////////////////////
//	Constants
///////////////////////////

#define LIGHT_TYPE_POINT		0
#define LIGHT_TYPE_SPOT			1
#define LIGHT_TYPE_DIRECTIONAL	2

#define MAX_LIGHTS_PER_CLUSTER	100

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

struct LightInfo
{
	float3 WorldPosition;
	float AttnRadius;

	float3 Color;
	float Brightness;

	uint Type;
	float3 Direction;

	float SpotFOV;
	float3 _pad;
};

///////////////////////////
//	Constant Buffers
///////////////////////////

cbuffer LightConstants : register( b0 )
{
	uint totalLightCount;
	uint3 _pad_lc_1;

	matrix ViewMatrix;
}

///////////////////////////
//	Resources
///////////////////////////

StructuredBuffer< ClusterInfo > clusterInfoList		: register( t0 );
StructuredBuffer< LightInfo > lightInfoList			: register( t1 );

///////////////////////////
//	Outputs
///////////////////////////

RWTexture3D< uint2 > clusterLightIndicies	: register( u0 );
RWBuffer< uint > lightIndexList				: register( u1 );

///////////////////////////
//	Shared Memory
///////////////////////////

groupshared uint activeLights[ 100 ];
groupshared uint activeLightCount = 0;

//////////////////////////////
//	Function Declarations
//////////////////////////////

bool checkPointLight( float3 inLightOrigin, float inLightRadius, float3 minAABB, float3 maxAABB );
bool checkSpotLight( float3 inLightOrigin, float inLightMaxDist, float3 inLightDirection, float inLightFOV, float3 minAABB, float3 maxAABB );

///////////////////////////
//	Entry Point
///////////////////////////

[numthreads( 512, 1, 1 )]
void main( uint3 localThreadID : SV_GroupThreadID, uint3 groupID : SV_GroupID )
{
	// Setup the variables we need
	uint lightBatch			= localThreadID.x;
	uint clusterX			= groupID.x;
	uint clusterY			= groupID.y;
	uint clusterZ			= groupID.z;
	uint clusterFlatIndex	= clusterX + ( clusterY * 15 ) + ( clusterZ * 150 );
	ClusterInfo cluster		= clusterInfoList[ clusterFlatIndex ];

	// Calculate which lights need to be checked by this thread, the list of lights in the scene are split among 512 threads (per cluster)
	uint lightsPerThread	= clamp( ( ( totalLightCount - 1 ) / 512 ) + 1, 1, 512 );
	uint lightBegin			= lightBatch * lightsPerThread;
	uint lightEnd			= lightBegin + lightsPerThread;


	if( lightBegin < totalLightCount && cluster.bActive )
	{
	// Clamp the end of our light list, to the end of the global light list, so we dont run past it
		lightEnd = min( lightEnd, totalLightCount );

		// Loop through our assigned lights
		for( uint index = lightBegin; index < lightEnd; index++ )
		{
			/*
			LightInfo light = lightInfoList[ index ];

			// Check for light collision based on the type of light
			bool bCollision = false;
			if( light.Type == LIGHT_TYPE_POINT )
			{
				bCollision = checkPointLight( light.WorldPosition, light.AttnRadius, cluster.minAABB, cluster.maxAABB );
			}
			else if( light.Type == LIGHT_TYPE_SPOT )
			{
				bCollision = checkSpotLight( light.WorldPosition, light.AttnRadius, light.Direction, light.SpotFOV, cluster.minAABB, cluster.maxAABB );
			}
			else if( light.Type == LIGHT_TYPE_DIRECTIONAL )
			{
				bCollision = true; // For now, directional lights affect ALL pixels
			}

			if( bCollision )
			{
				// Incrememnt cluster light counter
				uint lastIndex;
				InterlockedAdd( activeLightCount, 1, lastIndex );

				if( lastIndex >= MAX_LIGHTS_PER_CLUSTER ) { break; }

				// Add light to the temporary cluster light list
				activeLights[ lastIndex ] = index;
			}
			*/

			LightInfo light = lightInfoList[ index ];
			bool bCollision = checkPointLight( light.WorldPosition, light.AttnRadius, cluster.minAABB, cluster.maxAABB );

			if( bCollision )
			{
				uint lastIndex;
				InterlockedAdd( activeLightCount, 1, lastIndex );
				if( lastIndex >= MAX_LIGHTS_PER_CLUSTER ) { break; }

				activeLights[ lastIndex ] = index;
			}
		}
	}

	// Wait for all threads to reach this point
	DeviceMemoryBarrierWithGroupSync();
	//GroupMemoryBarrierWithGroupSync();
	//AllMemoryBarrierWithGroupSync();

	uint lightListOffset = clusterFlatIndex * MAX_LIGHTS_PER_CLUSTER;

	// Use one of the threads only to write the entry for this cluster to the indicies list
	if( lightBatch == 0 )
	{
		clusterLightIndicies[ uint3( clusterX, clusterY, clusterZ ) ] = uint2( lightListOffset, activeLightCount );
	}

	// Divide the task of writing light identifiers to the index list, one per thread
	if( lightBatch < activeLightCount )
	{
		lightIndexList[ lightListOffset + lightBatch ] = activeLights[ lightBatch ];
	}

}

///////////////////////////
//	Helper Functions
///////////////////////////

bool checkPointLight( float3 inLightOrigin, float inLightRadius, float3 minAABB, float3 maxAABB )
{
	// Transform light to view space, so we can check against cluster bounds
	// TODO: Still having the ocasional black square flicker
	// Idea for one solution, is adding a small value to each bounding box, to make a 'true' result a little easier
	// We just want to be absolutley sure that we arent clipping lights that should be active
	float4 originPr = float4( inLightOrigin, 1.f );
	originPr = mul( originPr, ViewMatrix );

	float minDist = 0.f;

	if( originPr.x < minAABB.x )		{ minDist += ( ( originPr.x - minAABB.x ) * ( originPr.x - minAABB.x ) ); }
	else if( originPr.x > maxAABB.x )	{ minDist += ( ( originPr.x - maxAABB.x ) * ( originPr.x - maxAABB.x ) ); }

	if( originPr.y < minAABB.y )		{ minDist += ( ( originPr.y - minAABB.y ) * ( originPr.y - minAABB.y ) ); }
	else if( originPr.y > maxAABB.y )	{ minDist += ( ( originPr.y - maxAABB.y ) * ( originPr.y - maxAABB.y ) ); }

	if( originPr.z < minAABB.z )		{ minDist += ( ( originPr.z - minAABB.z ) * ( originPr.z - minAABB.z ) ); }
	else if( originPr.z > maxAABB.z )	{ minDist += ( ( originPr.z - maxAABB.z ) * ( originPr.z - maxAABB.z ) ); }

	return( minDist <= ( inLightRadius * inLightRadius ) );
}


bool checkSpotLight( float3 inLightOrigin, float inLightMaxDist, float3 inLightDirection, float inLightFOV, float3 minAABB, float3 maxAABB )
{
	// TODO
	// Were going to convert the AABB to a sphere, and do a sphere-cone collision
	return true;
}