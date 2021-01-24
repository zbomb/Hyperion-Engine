//
//	Hyperion Engine Shader
//	Lighting Pixel Shader
//	© 2021, Zachary Berry
//

// Constants
#define MAX_LIGHT_COUNT 50


// Structures
struct PixelInput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
};


struct LightInfo
{
	float3 WorldPosition;
	float AttnRadius;
	float3 Color;
	float Brightness;

	// ------ 32 -------
	float4 _pad;
	// ------ 48 --------
};

struct Cluster
{
	float4 minAABB;
	float4 maxAABB;
};


// Textures
Texture2D diffuseTexture	: register( t0 );
Texture2D normalTexture		: register( t1 );
Texture2D specularTexture	: register( t2 );


// Samplers
SamplerState pointSampler : register( s0 );


// Constant Buffers
cbuffer CameraInfo : register( b0 )
{
	float3 cameraPosition;
	float _c_pad_4;
	float _c_pad_5;
	float _c_pad_1;
	float _c_pad_2;
	float _c_pad_3;
	matrix invViewProjMatrix;
};


cbuffer LightList : register( b1 )
{
	float3 ambientColor;
	float ambientIntensity;
	// ----- 16 ---------
	int lightCount;
	float3 _ll_padding;

	LightInfo lightArray[ MAX_LIGHT_COUNT ];
};


StructuredBuffer< Cluster > clusterList : register( t3 );


// Shader Program
float4 main( PixelInput input ) : SV_TARGET
{
	float4 colorRough;
	float4 normalDepth;
	float4 specular;
	float4 outputColor;

	// Sample the textures in the g-buffer
	colorRough			= diffuseTexture.Sample( pointSampler, input.TexCoord );
	normalDepth			= normalTexture.Sample( pointSampler, input.TexCoord );
	specular			= specularTexture.Sample( pointSampler, input.TexCoord );
	float4 color		= float4( colorRough.xyz, 1.f );
	float3 normal		= normalDepth.xyz;
	float roughness		= colorRough.w;
	float depth			= normalDepth.w;

	// Calculate world space position of this pixel
	float4 pixelWorldPos = float4( input.TexCoord.x * 2.f - 1.f, ( 1.f - input.TexCoord.y ) * 2.f - 1.f, depth, 1.f );
	pixelWorldPos = mul( pixelWorldPos, invViewProjMatrix );
	pixelWorldPos /= pixelWorldPos.w;

	// Accumulate light info to calculate a final color
	float4 lightComponent = float4( 0.f, 0.f, 0.f, 0.f );

	for( int i = 0; i < lightCount; i++ )
	{
		// Calculate light intensity
		float dist				= abs( distance( pixelWorldPos.xyz, lightArray[ i ].WorldPosition ) );
		float3 viewDir			= normalize( cameraPosition - pixelWorldPos.xyz );
		float attnRadius		= lightArray[ i ].AttnRadius;
		float intensity			= saturate( lightArray[ i ].Brightness * ( ( attnRadius * attnRadius ) / ( attnRadius * attnRadius + dist * dist ) ) );
		float3 lightDir			= -normalize( lightArray[ i ].WorldPosition - pixelWorldPos.xyz );

		// If light intensity is less than 1/256, skip this light
		if( intensity < 0.0039f ) { continue; }

		// Calculate diffuse component
		float diffI = saturate( dot( normal, -lightDir ) );
		
		// Calculate specular component
		//float3 specR = normalize( 2.f * diffI * normal + lightDir );
		//float specI = pow( saturate( dot( specR, viewDir ) ), specular.w );

		lightComponent += ( float4( lightArray[ i ].Color, 1.f ) * diffI * intensity );
		//lightComponent += ( specI * float4( specular.xyz, 1.f ) * intensity );
	}

	// Add ambient lighting
	lightComponent += ( float4( ambientColor, 1.f ) * ambientIntensity );
	return saturate( color * lightComponent );
}