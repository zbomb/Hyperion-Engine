//
//	Hyperion Engine Shader
//	GBuffer Pixel Shader
//	© 2021, Zachary Berry
//

// Types
struct PixelInput
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD0;
};


struct PixelOutput
{
	float4 Color : SV_Target0; // Output to the first texture in G-Buffer
	float4 Normal : SV_Target1; // Output to the second texture in G-Buffer
	float4 Specular : SV_Target2; // Output to the third texture in G-Buffer
};


struct ClusterInfo
{
	float4 minAABB;
	float4 maxAABB;
	bool bActive;
	float3 _pad;
};


// Resources
Texture2D shaderTexture : register( t0 );
SamplerState wrapSampler : register( s0 );
RWStructuredBuffer< ClusterInfo > clusterList : register( u3 );

// Constant Buffers
cbuffer clusterInfoBuffer
{
	float DepthCalcTermA;
	float DepthCalcTermB;
	float2 ClusterSize;
	uint2 ClusterCount;
	float2 _ci_pad_1;
};



// Function prototypes
float4 calculateColorAndRoughness( float2 inTexCoord );
float4 calculateSpecularity( float2 inTexCoord );
void markActiveCluster( float3 inPixelPosition );


// Shader Program
PixelOutput main( PixelInput input )
{
	PixelOutput output;

	// First, calculate unlit pixel color and roughness
	output.Color = calculateColorAndRoughness( input.TexCoord );

	// Pass through the normal direction, and depth
	output.Normal = float4( input.Normal, input.Position.z );

	// Calculate specular color
	output.Specular = calculateSpecularity( input.TexCoord );

	// Mark the cluster that contains this pixel as active
	markActiveCluster( input.Position.xyz );

	return output;
}


// Helper functions
float4 calculateColorAndRoughness( float2 inTexCoord )
{
	return shaderTexture.Sample( wrapSampler, inTexCoord );
}


float4 calculateSpecularity( float2 inTexCoord )
{
	return float4( 1.f, 1.f, 1.f, 1.f );
}


void markActiveCluster( float3 inPixelPosition )
{
	// First, lets calculate the depth slice this pixel is contained within
	// DepthCalcTermA: #depthSlices / log( far / near )
	// DepthCalcTermB: ( #depthSlices * log( near ) ) / log( far / near ) or DepthCalcTermA * log( near )
	uint depthSlice = floor( log( inPixelPosition.z ) * DepthCalcTermA - DepthCalcTermB );
	uint3 clusterIndex = uint3( floor( inPixelPosition.x / ClusterSize.x ), floor( inPixelPosition.y / ClusterSize.y ), depthSlice );

	// Now, we need to calculate the 'flat index'
	uint flatClusterIndex = clusterIndex.x + ( clusterIndex.y * ClusterCount.x ) + clusterIndex.z * ( ClusterCount.x * ClusterCount.y );

	// And finally, lets mark it as 'active'
	clusterList[ flatClusterIndex ].bActive = true;
}