////////////////////////////////////////////////////
//		Hyperion Engine	
//		Forward Pixel Shader	
//		© 2021, Zachary Berry
////////////////////////////////////////////////////

///////////////////////////
//	Constant Values
///////////////////////////

#define LIGHT_MIN_INTENSITY 0.0039f

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

// TODO: Share constant buffer with deferred lighting shader (small gain)
cbuffer RenderInfoBuffer : register( b0 )
{
	float3 CameraPosition;
	float _pad_vb_1;

	float ProjectionTermA;
	float ProjectionTermB;
	float ScreenNear;
	float ScreenFar;

	float ScreenWidth;
	float ScreenHeight;
	float DepthSliceTermA;
	float DepthSliceTermB;

	float3 AmbientColor;
	float AmbientIntensity;

	matrix InverseViewMatrix;
}

///////////////////////////
//	Resources
///////////////////////////

Texture2D baseMapTexture : register( t0 );
SamplerState wrapSampler : register( s0 );

StructuredBuffer< ClusterInfo > clusterInfoList		: register( t1 );
Texture3D< uint2 > clusterLightIndicies				: register( t2 );
Buffer< uint > lightIndexList						: register( t3 );
StructuredBuffer< LightInfo > lightInfoList			: register( t4 );

//////////////////////////////
//	Function Declarations
//////////////////////////////

uint3 getClusterIndicies( float2 inPixelPosition, float inDepth );

///////////////////////////
//	Entry Point
///////////////////////////

float4 main( PixelInput input ) : SV_TARGET
{
	float viewSpaceDepth = input.VSPosition.z;

	// Sample texture
	float4 diffuseColor = baseMapTexture.Sample( wrapSampler, input.TexCoords );

	// Reconstruct world space position

	float4 viewPos = float4( input.VSPosition, 1.f );
	float4 worldPos = mul( viewPos, InverseViewMatrix );
	worldPos /= worldPos.w;

	// Calculate the direction were viewing this pixel from
	float3 viewDir = normalize( CameraPosition - worldPos.xyz );

	// Calculate which cluster this pixel is within
	uint3 clusterIndex = getClusterIndicies( input.Position.xy, viewSpaceDepth );
	uint flatClusterIndex = clusterIndex.x + ( clusterIndex.y * 15 ) + ( clusterIndex.z * 150 );

	// Get lighting info for this cluster
	uint2 lightAssignmentInfo = clusterLightIndicies[ clusterIndex ];
	uint lightOffset = lightAssignmentInfo.x;
	uint lightCount = lightAssignmentInfo.y;

	// Calculate lighting contribution from each active light
	float4 lightComponent = float4( 0.f, 0.f, 0.f, 0.f );

	for( uint i = 0; i < lightCount; i++ )
	{
		// Get info about the current light
		LightInfo light = lightInfoList[ lightIndexList[ lightOffset + i ] ];

		// Calculate distance and direction from pixel to light source
		float lightDist = abs( distance( worldPos.xyz, light.WorldPosition ) );
		float3 lightDir = -normalize( light.WorldPosition - worldPos.xyz );

		// Calculate light intensity
		float lightIntensity = saturate( 1.f - ( lightDist * lightDist ) / ( light.AttnRadius * light.AttnRadius ) );
		lightIntensity *= lightIntensity;

		// If intensity is less than 1/256, we ignore this light
		if( lightIntensity > LIGHT_MIN_INTENSITY )
		{
			// Calculate the diffuse component
			float diffuseIntensity = saturate( dot( input.Normal, -lightDir ) );
			float4 diffuseContribution = float4( light.Color, 1.f ) * diffuseIntensity * lightIntensity;

			// We want to 'clamp' the bottom end of the diffuse lighting contribution
			diffuseContribution.x = max( diffuseContribution.x, 0.f );
			diffuseContribution.y = max( diffuseContribution.y, 0.f );
			diffuseContribution.z = max( diffuseContribution.z, 0.f );
			diffuseContribution.w = max( diffuseContribution.w, 0.f );

			lightComponent += diffuseContribution;

			// TODO: Calculate specular component
			//float3 specR = normalize( 2.f * diffI * normal + lightDir );
			//float specI = pow( saturate( dot( specR, viewDir ) ), specular.w );
			//lightComponent += ( specI * float4( specular.xyz, 1.f ) * intensity );
		}
	}

	// Add in the ambient light term
	lightComponent += ( float4( AmbientColor, 1.f ) * AmbientIntensity );
	return saturate( diffuseColor * lightComponent );
}

//////////////////////////////
//	Function Definitions
//////////////////////////////

uint3 getClusterIndicies( float2 inPixelPosition, float inDepth )
{
	// Calculate the cluster width and height
	float clusterSizeX = ScreenWidth / 15.f;
	float clusterSizeY = ScreenHeight / 10.f;

	// Calculate the cluster X and Y
	uint clusterX = uint( floor( inPixelPosition.x / clusterSizeX ) );
	uint clusterY = uint( floor( inPixelPosition.y / clusterSizeY ) );

	// Calculate the cluster depth slice (Z)
	uint clusterZ = uint( floor( log( inDepth ) * DepthSliceTermA - DepthSliceTermB ) );

	return uint3( clusterX, clusterY, clusterZ );
}