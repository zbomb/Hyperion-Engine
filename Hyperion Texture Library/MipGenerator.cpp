/*-------------------------------------------------------------------------------------------------
	Hyperion - Texture Library
	© 2020 - Zack Berry
---------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "MipGenerator.h"
#include "CMP_Framework.h"

using namespace System;

constexpr auto HYPERION_MAX_TEXTURE_SIZE_PX = 16384;


namespace Hyperion
{

	MipGeneratorResult MipGenerator::Generate( Request^ inParams, [Out] Dictionary< Byte, MIP^ >^% outData )
	{
		outData = nullptr;

		// So, we basically would need to validate the parameters, convert to CMP parameters, run the algorithm and return a result
		// First, check the data and resolutions
		if( inParams == nullptr ||
			inParams->Data == nullptr ||
			inParams->Data->Length <= 0 ||
			inParams->Width * inParams->Height * 4 != inParams->Data->Length )
		{
			return MipGeneratorResult::InvalidSourceData;
		}
		else if( inParams->Width == 1 && inParams->Height == 1 ||
				 inParams->Width > HYPERION_MAX_TEXTURE_SIZE_PX || inParams->Height > HYPERION_MAX_TEXTURE_SIZE_PX )
		{
			return MipGeneratorResult::InvalidSourceResolution;
		}

		// Now, the parameters seem valid, lets start using compressonator
		// First, we need to create the input mip set to the algorithm
		CMP_MipSet mipSet;
		memset( &mipSet, 0, sizeof( CMP_MipSet ) );

		// Allocate Mip Set
		mipSet.m_nWidth = inParams->Width;
		mipSet.m_nHeight = inParams->Height;
		mipSet.m_nDepth = 1;
		mipSet.m_ChannelFormat = CMP_ChannelFormat::CF_8bit;
		mipSet.m_TextureDataType = CMP_TextureDataType::TDT_ARGB;
		mipSet.m_TextureType = CMP_TextureType::TT_2D;
		mipSet.m_nMipLevels = 1;
		mipSet.m_format = CMP_FORMAT::CMP_FORMAT_RGBA_8888;

		// Calculate maximum number of mip levels to generate
		int maxMipLevels = 0;
		int nW = inParams->Width;
		int nH = inParams->Height;

		while( nW >= 1 || nH >= 1 )
		{
			maxMipLevels++;

			if( nW == 1 || nH == 1 ) { break; }

			nW = nW > 1 ? nW >> 1 : 1;
			nH = nH > 1 ? nH >> 1 : 1;
		}

		mipSet.m_nMaxMipLevels = maxMipLevels;
		if( maxMipLevels <= 1 ) // Should never bee true, basically checked earlier in a different way
		{
			return MipGeneratorResult::InvalidSourceResolution;
		}

		// Allocate Mip Level Table
		// Were only going to worry about the source
		//mipSet.m_pMipLevelTable = reinterpret_cast<CMP_MipLevelTable*>( calloc( 1, sizeof( CMP_MipLevel* ) ) );
		//mipSet.m_pMipLevelTable = reinterpret_cast<CMP_MipLevelTable*>( calloc( mipSet.m_nMaxMipLevels, sizeof( CMP_MipLevel* ) ) );
		mipSet.m_pMipLevelTable = new CMP_MipLevelTable[ mipSet.m_nMaxMipLevels ];
		//if( mipSet.m_pMipLevelTable == nullptr ) throw gcnew Exception( "Failed to allocated mip level table" );

		for( int i = 0; i < mipSet.m_nMaxMipLevels; i++ )
		{
			mipSet.m_pMipLevelTable[ i ] = new CMP_MipLevel();
			memset( mipSet.m_pMipLevelTable[ i ], 0, sizeof( CMP_MipLevel ) );

			// If we fail to allocate, free the rest of the structure before failing
			if( !mipSet.m_pMipLevelTable[ i ] )
			{
				for( i -= 1; i >= 0; i-- )
				{
					if( mipSet.m_pMipLevelTable[ i ] )
					{
						delete mipSet.m_pMipLevelTable[ i ];
						mipSet.m_pMipLevelTable[ i ] = nullptr;
					}
				}

				delete[] mipSet.m_pMipLevelTable;
				mipSet.m_pMipLevelTable = nullptr;

				return MipGeneratorResult::Failed;
			}
		}

		// We want to load the existing data into the mip level table
		CMP_MipLevel* baseLevel = mipSet.m_pMipLevelTable[ 0 ];

		baseLevel->m_dwLinearSize = inParams->Width * inParams->Height * 4;
		baseLevel->m_nWidth = inParams->Width;
		baseLevel->m_nHeight = inParams->Height;

		baseLevel->m_pbData = new unsigned char[ baseLevel->m_dwLinearSize ];
		Marshal::Copy( inParams->Data, 0, IntPtr( (void*) baseLevel->m_pbData ), baseLevel->m_dwLinearSize );

		baseLevel->m_pcData = reinterpret_cast<CMP_COLOR*>( baseLevel->m_pbData );
		baseLevel->m_pdwData = reinterpret_cast<CMP_DWORD*>( baseLevel->m_pbData );
		baseLevel->m_pfData = reinterpret_cast<CMP_FLOAT*>( baseLevel->m_pbData );
		baseLevel->m_phfsData = reinterpret_cast<CMP_HALFSHORT*>( baseLevel->m_pbData );
		baseLevel->m_pvec8Data = reinterpret_cast<CMP_VEC8*>( baseLevel->m_pbData );
		baseLevel->m_pwData = reinterpret_cast<CMP_WORD*>( baseLevel->m_pbData );

		//CMP_INT minSize = CMP_CalcMinMipSize( inParams->Height, inParams->Width, 0 );
		CMP_INT minSize = 1;

		// Generate new mip levels
		CMP_GenerateMIPLevels( &mipSet, minSize );

		outData = gcnew Dictionary< Byte, MIP^ >();

		// Now, we need to get the output mip levels into a format we can send back to C#
		for( int level = 0; level < mipSet.m_nMipLevels; level++ )
		{
			// Read the mip from the set
			CMP_MipLevel* mip = mipSet.m_pMipLevelTable[ level ];

			// Ensure its valid
			if( mip == nullptr ||
				mip->m_pbData == nullptr ||
				mip->m_nWidth == 0 ||
				mip->m_nHeight == 0 )
			{
				CMP_FreeMipSet( &mipSet );
				return MipGeneratorResult::GenerateFailed;
			}

			// Create output entry and copy data into it
			auto newMip = gcnew MipGenerator::MIP();
			newMip->Index = (Byte) level;
			newMip->Width = mip->m_nWidth;
			newMip->Height = mip->m_nHeight;
			newMip->Data = gcnew cli::array< Byte >( mip->m_dwLinearSize );
			
			Marshal::Copy( IntPtr( (void*) mip->m_pbData ), newMip->Data, 0, mip->m_dwLinearSize );

			// Add to output map
			outData->Add( newMip->Index, newMip );
		}

		CMP_FreeMipSet( &mipSet );
		return MipGeneratorResult::Success;
	}

}