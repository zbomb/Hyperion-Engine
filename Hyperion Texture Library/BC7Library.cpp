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
		if( inWidth == 0 || inHeight == 0 ||( inData->Length != ( inWidth * inHeight * 4 ) ) )
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
		//options.bUseCGCompress = true;
		options.nEncodeWith = CMP_GPU;
		options.fquality = 0.01f;
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

		// Since compressonator doesnt seem to calculate the row pitch for us, were going to do it manually
		// So.. we could... declare that a row is totalDataSize / ( height / 4 )
		// This is because, were doing 4x4 blocks, so a row, is actually 4 rows of pixels
		// Each 4x4 block is 16 bytes, this means the padding is already factored in and the calculation is easier
		// But first, lets round the height up to the nearest multiple of 4
		uint32_t paddedHeight = inHeight;
		while( paddedHeight % 4 != 0 )
		{
			paddedHeight++;
		}

		// Sanity Check
		if( outData->Length % ( paddedHeight / 4 ) != 0 )
		{
			return BC7Result::PitchCalcError;
		}

		outRowSize = outData->Length / ( paddedHeight / 4 );

		return BC7Result::Success;
	}



	BC7Result BC7Library::Decode( cli::array< Byte >^ inData, UInt32 inWidth, UInt32 inHeight, UInt32 inPitch, bool bIncludeAlpha, [ Out ] cli::array< Byte >^% outData )
	{
		if( inWidth == 0 || inHeight == 0 || inData->Length == 0 || inPitch == 0 )
		{
			outData = nullptr;
			return BC7Result::InvalidParams;
		}

		CMP_Texture inTexture;
		inTexture.dwSize = sizeof( inTexture );
		inTexture.dwWidth = inWidth;
		inTexture.dwHeight = inHeight;
		inTexture.dwPitch = inPitch;
		inTexture.format = CMP_FORMAT_BC7;

		pin_ptr< Byte > sourcePtr = &inData[ 0 ];
		inTexture.pData = sourcePtr;
		inTexture.dwDataSize = inData->Length;

		CMP_Texture outTexture;
		outTexture.dwSize = sizeof( outTexture );
		outTexture.dwWidth = inWidth;
		outTexture.dwHeight = inHeight;
		outTexture.dwPitch = 0;
		outTexture.format = CMP_FORMAT_RGBA_8888;
		outTexture.dwDataSize = CMP_CalculateBufferSize( &outTexture );

		outData = gcnew cli::array< Byte >( outTexture.dwDataSize );
		pin_ptr< Byte > outPtr = &outData[ 0 ];

		outTexture.pData = outPtr;

		CMP_CompressOptions options ={ 0 };
		options.dwSize = sizeof( options );
		options.fquality = 0.25f;
		options.dwnumThreads = 8;

		CMP_ERROR err = CMP_ConvertTexture( &inTexture, &outTexture, &options, NULL );
		if( err != CMP_OK )
		{
			outData	= nullptr;
			return (BC7Result) err;
		}

		return BC7Result::Success;
	}


}