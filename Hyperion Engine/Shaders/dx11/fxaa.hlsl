////////////////////////////////////////////////////
//		Hyperion Engine	
//		FXAA Post-Processing Shader	
//		© 2021, Zachary Berry
////////////////////////////////////////////////////

#define FXAA_PC 1
#define FXAA_HLSL_5 1
#define FXAA_GREEN_AS_LUMA 0


#include "../incl/FXAA.h"

///////////////////////////
//	Structures
///////////////////////////

struct PixelInput
{
	float4 Position		: SV_POSITION;
	float2 TexCoords	: TEXCOORD;
};

///////////////////////////
//	Resources
///////////////////////////

Texture2D screenBuffer		: register( t0 );
SamplerState pointSampler	: register( s0 );

///////////////////////////
//	Constant Buffer
///////////////////////////

cbuffer FXAABuffer : register( b0 )
{
	float ScreenWidth;
	float ScreenHeight;
	float2 _pad_fb_1;
}

///////////////////////////
//	Entry Point
///////////////////////////

float4 main( PixelInput input ) : SV_Target
{
	FxaaTex sourceTex	= { pointSampler, screenBuffer };
	float2 rcpFrame		= float2( 1.f / ScreenWidth, 1.f / ScreenHeight );

	// Run FXAA 
	return FxaaPixelShader(
		input.TexCoords,
		float4( 0.f, 0.f, 0.f, 0.f ),	// Console Only
		sourceTex,
		sourceTex,						// Console Only
		sourceTex,						// Console Only
		rcpFrame,
		float4( 0.f, 0.f, 0.f, 0.f ),	// Console Only
		float4( 0.f, 0.f, 0.f, 0.f ),	// Console Only
		float4( 0.f, 0.f, 0.f, 0.f ),	// Console Only
		0.75f,
		0.166f,
		0.0833f,
		0.f,							// Console Only
		0.f,							// Console Only
		0.f,							// Console Only
		float4( 0.f, 0.f, 0.f, 0.f )	// Console Only
	);
}