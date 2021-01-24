//
//	Hyperion Engine Shader
//	Cluster Compress Compute Shader
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
StructuredBuffer< ClusterInfo > clusterList : register( t0 );
AppendStructuredBuffer< uint > activeClusterList : register( u0 );


// Entry Point
[numthreads(15, 10, 1)]
void main( uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID )
{
	uint clusterX	= groupThreadID.x;
	uint clusterY	= groupThreadID.y;
	uint clusterZ	= groupID.x;

	uint flatIndex = clusterX + ( clusterY * 15 ) + ( clusterZ * 150 );
	if( clusterList[ flatIndex ].bActive )
	{
		activeClusterList.Append( flatIndex );
	}
}