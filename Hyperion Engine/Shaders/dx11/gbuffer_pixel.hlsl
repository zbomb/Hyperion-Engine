////////////////////////////////////////////////////
//		Hyperion Engine	
//		GBuffer Pixel Shader	
//		© 2021, Zachary Berry
////////////////////////////////////////////////////


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


struct PixelOutput
{
	float4 Color		: SV_Target0;
	float4 Normal		: SV_Target1;
	float4 Specular		: SV_Target2;
};

///////////////////////////
//	Resources
///////////////////////////

Texture2D baseMapTexture : register( t0 );
SamplerState wrapSampler : register( s0 );

///////////////////////////////
//	Function Declarations
///////////////////////////////

float4 calculateColorAndRoughness( float2 inTexCoord );
float4 calculateSpecularity( float2 inTexCoord );

///////////////////////////////
//	Entry Point
///////////////////////////////

PixelOutput main( PixelInput input )
{
	PixelOutput output;

	// Calculate diffuse color and roughness
	output.Color = calculateColorAndRoughness( input.TexCoords );

	// Pass through normal and depth
	output.Normal = float4( input.Normal, input.VSPosition.z );

	// Calculate specular color
	output.Specular = calculateSpecularity( input.TexCoords );

	return output;
}

///////////////////////////////
//	Helper Functions
///////////////////////////////

float4 calculateColorAndRoughness( float2 inTexCoord )
{
	return baseMapTexture.Sample( wrapSampler, inTexCoord );
}


float4 calculateSpecularity( float2 inTexCoord )
{
	return float4( 1.f, 1.f, 1.f, 1.f );
}