//
//	Hyperion Engine Shader
//	GBuffer Pixel Shader
//	© 2021, Zachary Berry
//

// Textures
Texture2D shaderTexture : register( t0 );


// Sampler States
SamplerState wrapSampler : register( s0 );


// Types
struct PixelInput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 Normal : NORMAL;
};


struct PixelOutput
{
	float4 Color : SV_Target0; // Output to the first texture in G-Buffer
	float4 Normal : SV_Target1; // Output to the second texture in G-Buffer
	float4 Specular : SV_Target2; // Output to the third texture in G-Buffer
};


// Shader Program
PixelOutput main( PixelInput input )
{
	PixelOutput output;

	// Sample texture color
	output.Color = shaderTexture.Sample( wrapSampler, input.TexCoord );

	// Pass through normal and depth
	output.Normal = float4( input.Normal, input.Position.w );

	// For now, were going to set specular to 0
	output.Specular = float4( 0.0f, 0.0f, 0.0f, 0.0f );

	return output;
}