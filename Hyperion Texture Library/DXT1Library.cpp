/*-------------------------------------------------------------------------------------------------
	Hyperion - Texture Library
	© 2020 - Zack Berry
---------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "DXT1Library.h"
#include "Compressonator.h"

using namespace System;


namespace Hyperion
{

	DXT1Result DXT1Library::Encode( cli::array< Byte >^ inData, UInt32 inWidth, UInt32 inHeight, [ Out ] cli::array< Byte >^% outData, [ Out ] UInt32% outRowSize )
	{
		if( inWidth == 0 || inHeight == 0 || ( inData->Length != ( inWidth * inHeight * 4 ) ) )
		{
			outData = nullptr;
			outRowSize = 0;
			return DXT1Result::InvalidParams;
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
		outTexture.format = CMP_FORMAT_DXT1;
		outTexture.dwDataSize = CMP_CalculateBufferSize( &outTexture );

		outData = gcnew cli::array< Byte >( outTexture.dwDataSize );
		pin_ptr< Byte > outPtr = &outData[ 0 ];

		outTexture.pData = outPtr;

		CMP_CompressOptions options ={ 0 };
		options.dwSize = sizeof( options );
		options.fquality = 1.0f;
		options.dwnumThreads = 8;
		options.bDXT1UseAlpha = false; // We are not going to use alpha (would be 1-bit anyway) with DXT-1

		CMP_ERROR err = CMP_ConvertTexture( &inTexture, &outTexture, &options, NULL );
		if( err != CMP_OK )
		{
			// There was an error
			outRowSize		= 0;
			outData			= nullptr;

			return (DXT1Result) err;
		}

		// Calculate row pitch, since compressonator doesnt seem to do this for us
		uint32_t paddedHeight = inHeight;
		while( paddedHeight % 4 != 0 )
		{
			paddedHeight++;
		}

		// Sanity Check
		if( outData->Length % ( paddedHeight / 4 ) != 0 )
		{
			return DXT1Result::PitchCalcError;
		}

		outRowSize = outData->Length / ( paddedHeight / 4 );
		return DXT1Result::Success;
	}


	DXT1Result DXT1Library::Decode( cli::array< Byte >^ inData, UInt32 inWidth, UInt32 inHeight, UInt32 inPitch, [ Out ] cli::array< Byte >^% outData )
	{
		if( inWidth == 0 || inHeight == 0 || inData->Length == 0 || inPitch == 0 )
		{
			outData = nullptr;
			return DXT1Result::InvalidParams;
		}

		CMP_Texture inTexture;
		inTexture.dwSize = sizeof( inTexture );
		inTexture.dwWidth = inWidth;
		inTexture.dwHeight = inHeight;
		inTexture.dwPitch = inPitch;
		inTexture.format = CMP_FORMAT_DXT1;

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
		options.fquality = 1.0f;
		options.dwnumThreads = 8;

		CMP_ERROR err = CMP_ConvertTexture( &inTexture, &outTexture, &options, NULL );
		if( err != CMP_OK )
		{
			outData	= nullptr;
			return (DXT1Result) err;
		}

		return DXT1Result::Success;
	}

}