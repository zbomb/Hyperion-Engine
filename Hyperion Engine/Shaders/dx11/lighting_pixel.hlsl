////////////////////////////////////////////////////
//		Hyperion Engine	
//		Lighting Pixel Shader	
//		© 2021, Zachary Berry
////////////////////////////////////////////////////

///////////////////////////
//	Constant Values
///////////////////////////

#define LIGHT_MIN_INTENSITY 0.0039f
#define SPECULARITY_INTENSITY 0.05f

///////////////////////////
//	Constant Buffers
///////////////////////////

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
//	Structures
///////////////////////////

struct PixelInput
{
	float4 Position		: SV_POSITION;
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
//	GBuffer Textures
///////////////////////////

Texture2D diffuseTexture	: register( t0 );
Texture2D normalTexture		: register( t1 );
Texture2D specularTexture	: register( t2 );

///////////////////////////
//	Samplers
///////////////////////////

SamplerState pointSampler : register( s0 );

///////////////////////////
//	Resources
///////////////////////////

StructuredBuffer< ClusterInfo > clusterInfoList		: register( t3 );
Texture3D< uint2 > clusterLightIndicies				: register( t4 );
Buffer< uint > lightIndexList						: register( t5 );
StructuredBuffer< LightInfo > lightInfoList			: register( t6 );


//////////////////////////////
//	Function Declarations
//////////////////////////////

uint3 getClusterIndicies( float2 inPixelPosition, float inDepth );
float getPixelLuminance( float3 inColor );

///////////////////////////
//	Entry Point
///////////////////////////

float4 main( PixelInput input ) : SV_Target
{
	// Sample the G-Buffer values
	float4 colorRoughSample		= diffuseTexture.Sample( pointSampler, input.TexCoords );
	float4 normalDepthSample	= normalTexture.Sample( pointSampler, input.TexCoords );
	float4 specularSample		= specularTexture.Sample( pointSampler, input.TexCoords );

	float4 diffuseColor		= float4( colorRoughSample.xyz, 1.f );
	float surfaceRoughness	= colorRoughSample.w;
	float3 surfaceNormal	= normalDepthSample.xyz;
	float viewSpaceDepth	= normalDepthSample.w;

	// Reconstruct view space position
	float4 viewPos = float4(
		viewSpaceDepth * ( input.TexCoords.x * 2.f - 1.f ) / ProjectionTermA,
		viewSpaceDepth * ( ( 1.f - input.TexCoords.y ) * 2.f - 1.f ) / ProjectionTermB,
		viewSpaceDepth,
		1.f
	);
	
	// Reconstruct world space position
	float4 worldPos = mul( viewPos, InverseViewMatrix );
	worldPos /= worldPos.w;

	// Calculate the direction were viewing this pixel from
	float3 viewDir = normalize( CameraPosition - worldPos.xyz );

	// We need to get the light list to factor in for this pixel
	uint lightOffset	= 0;
	uint lightCount		= 0;

	// If the pixel is past the far plane, dont lookup light list...
	if( viewSpaceDepth <= ScreenFar )
	{
		// Calculate which cluster this pixel is within
		uint3 clusterIndex		= getClusterIndicies( input.Position.xy, viewSpaceDepth );
		uint flatClusterIndex	= clusterIndex.x + ( clusterIndex.y * 15 ) + ( clusterIndex.z * 150 );

		// Get lighting info for this cluster
		uint2 lightAssignmentInfo	= clusterLightIndicies[ clusterIndex ];
		lightOffset					= lightAssignmentInfo.x;
		lightCount					= lightAssignmentInfo.y;
	}

	// Calculate lighting contribution from each active light
	float4 lightComponent = float4( 0.f, 0.f, 0.f, 0.f );

	for( uint i = 0; i < lightCount; i++ )
	{
		// Get info about the current light
		LightInfo light = lightInfoList[ lightIndexList[ lightOffset + i ] ];

		// Calculate distance and direction from pixel to light source
		float lightDist		= abs( distance( worldPos.xyz, light.WorldPosition ) );
		float3 lightDir		= -normalize( light.WorldPosition - worldPos.xyz );

		// Calculate light intensity
		float lightIntensity = saturate( 1.f - ( lightDist * lightDist ) / ( light.AttnRadius * light.AttnRadius ) );
		lightIntensity *= lightIntensity;

		// If intensity is less than 1/256, we ignore this light
		if( lightIntensity > LIGHT_MIN_INTENSITY )
		{
			// Calculate the diffuse component
			float diffuseIntensity		= saturate( dot( surfaceNormal, -lightDir ) );
			float4 diffuseContribution	= float4( light.Color, 1.f ) * diffuseIntensity * lightIntensity;

			// We want to 'clamp' the bottom end of the diffuse lighting contribution
			diffuseContribution.x = max( diffuseContribution.x, 0.f );
			diffuseContribution.y = max( diffuseContribution.y, 0.f );
			diffuseContribution.z = max( diffuseContribution.z, 0.f );
			diffuseContribution.w = max( diffuseContribution.w, 0.f );

			lightComponent += diffuseContribution;

			// TODO: Calculate specular component
			float3 specR	= normalize( 2.f * diffuseIntensity * surfaceNormal + lightDir );
			float specI		= pow( saturate( dot( specR, viewDir ) ), specularSample.w );
			lightComponent	+= ( specI * float4( light.Color, 1.f ) * lightIntensity * SPECULARITY_INTENSITY );
		}
	}

	// Add in the ambient light term
	lightComponent += ( float4( AmbientColor, 1.f ) * AmbientIntensity );

	float4 finalPixel = saturate( diffuseColor * lightComponent );

	// We want to calculate the luminance of the pixel, and store in the alpha channel
	// We use this during the post process pass
	finalPixel.a = getPixelLuminance( finalPixel.rgb );
	return finalPixel;

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

	return uint3( min( clusterX, 14 ), min( clusterY, 9 ), min( clusterZ, 23 ) );
}


float getPixelLuminance( float3 inColor )
{
	// Calculate luminance in linear color space
	float linearValue = inColor.r * 0.2126 + inColor.g * 0.7152 + inColor.b * 0.0722;
	return linearValue;
}