/*-------------------------------------------------------------------------------------------------
	Hyperion - Texture Library
	© 2020 - Zack Berry
---------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "BC7Library.h"
#include "Compressonator.h"

using namespace System;


namespace Hyperion
{
	/*
		This is an inter-layer between the C++ encoder we are using, and C# 
		So, really were just passing around the data and making the proper call
	*/
	BC7Result BC7Library::Encode( cli::array< Byte >^ inData, UInt32 inWidth, UInt32 inHeight, bool bIncludeAlpha, [Out] cli::array< Byte >^% outData, [Out] UInt32% outRowSize )
	{
		if( inWidth == 0 || inHeight == 0 || ( inData->Length != ( inWidth * inHeight * 4 ) ) )
		{
			outData = nullptr;
			outRowSize = 0;
			return BC7Result::InvalidParams;
		}

		CMP_Texture inTexture;
		inTexture.dwSize = sizeof( inTexture );
		inTexture.dwWidth = inWidth;
		inTexture.dwHeight = inHeight;
		inTexture.dwPitch = inWidth * 4;
		inTexture.format = CMP_FORMAT_RGBA_8888;
		
		pin_ptr< Byte > sourcePtr = &inData[ 0 ];
		inTexture.pData = sourcePtr;
		inTexture.dwDataSize = inData->Length;

		CMP_Texture outTexture;
		outTexture.dwSize = sizeof( outTexture );
		outTexture.dwWidth = inWidth;
		outTexture.dwHeight = inHeight;
		outTexture.dwPitch = 0;
		outTexture.format = CMP_FORMAT_BC7;
		outTexture.dwDataSize = CMP_CalculateBufferSize( &outTexture );
		
		outData = gcnew cli::array< Byte >( outTexture.dwDataSize );
		pin_ptr< Byte > outPtr = &outData[ 0 ];

		outTexture.pData = outPtr;

		CMP_CompressOptions options = { 0 };
		options.dwSize = sizeof( options );
		options.fquality = 0.05f;
		options.dwnumThreads = 8;

		// Were going to let the algorithm choose the compression mode used, but, we want to ensure if there
		// is no alpha, a mode that uses alpha doesnt get selected, so.. when bIncludeAlpha is false (i.e. color only)
		// then we want to set 'brestrictColour' to true, to ensure ONLY color info get encoded
		options.brestrictColour = !bIncludeAlpha;

		CMP_ERROR err = CMP_ConvertTexture( &inTexture, &outTexture, &options, NULL );
		if( err != CMP_OK )
		{
			// There was an error
			outRowSize		= 0;
			outData			= nullptr;

			return (BC7Result) err;
		}

		outRowSize = outTexture.dwPitch;
		return BC7Result::Success;
	}


}