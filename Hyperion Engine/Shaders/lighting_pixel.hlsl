//
//	Hyperion Engine Shader
//	Lighting Pixel Shader
//	© 2021, Zachary Berry
//


// Textures
Texture2D diffuseTexture : register( t0 );
Texture2D normalTexture : register( t1 );
Texture2D specularTexture : register( t2 );


// Samplers
SamplerState pointSampler : register( s0 );

// TODO: Lighting Buffer?

// Types
struct PixelInput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
};


// Shader Program
float4 main( PixelInput input ) : SV_TARGET
{
	float4 colorRough;
	float4 normalDepth;
	float4 specular;
	float4 outputColor;

	// Sample the textures in the g-buffer
	colorRough = diffuseTexture.Sample( pointSampler, input.TexCoord );
	normalDepth = normalTexture.Sample( pointSampler, input.TexCoord );
	specular = specularTexture.Sample( pointSampler, input.TexCoord );

	// Light direction debug
	float3 lightDirection = float3( 0.f, 0.f, -1.f );
	float lightIntensity = saturate( dot( normalDepth.xyz, lightDirection ) );

	float4 diffuseColor = float4( colorRough.x, colorRough.y, colorRough.z, 1.f );
	outputColor = saturate( diffuseColor * lightIntensity );

	return outputColor;
}