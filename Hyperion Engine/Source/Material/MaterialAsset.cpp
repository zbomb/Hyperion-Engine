/*==================================================================================================
	Hyperion Engine
	Source/Material/MaterialAsset.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Material/MaterialAsset.h"
#include "Hyperion/Material/MaterialInstance.h"
#include "Hyperion/Core/Library/Binary.h"


namespace Hyperion
{
	std::atomic< uint32 > MaterialAsset::m_lastInstanceIdentifier( 1 );


	std::shared_ptr< MaterialInstance > MaterialAsset::GetStaticInstance()
	{
		// Check if weve already created a static instance, if so return it
		if( m_StaticInstance )
		{
			return m_StaticInstance;
		}

		// Create material instance from this asset


		return nullptr;
	}


	std::shared_ptr< MaterialInstance > MaterialAsset::GetDynamicInstance()
	{
		return nullptr;
	}



	struct MaterialFileEntry
	{
		String Key;
		std::any Value;
	};

	struct MaterialFileHeader
	{
		uint16 EntryCount;
		uint8 bAllowTransparency;
		uint8 _rsvd_1_;
		uint32 _rsvd_2_;
		uint32 _rsvd_3_;
		uint32 _rsvd_4_;

		MaterialFileHeader()
			: EntryCount( 0 ), bAllowTransparency( 0 )
		{}
	};

	struct MaterialFile
	{
		MaterialFileHeader Header;
		std::vector< MaterialFileEntry > Entries;
	};

	enum class MaterialLineType
	{
		Int = 0,
		Float = 1,
		Bool = 2,
		String = 3,
		Texture = 4
	};


	bool ReadMaterialHeader( std::vector< byte >::const_iterator& Pos, bool bLittleEndian, MaterialFileHeader& Out )
	{
		// Read header entries in order
		Binary::DeserializeUInt16( Pos, Out.EntryCount, bLittleEndian );
		std::advance( Pos, 2 );

		Binary::DeserializeUInt8( Pos, Out.bAllowTransparency );
		std::advance( Pos, 1 );

		Binary::DeserializeUInt8( Pos, Out._rsvd_1_ );
		std::advance( Pos, 1 );

		Binary::DeserializeUInt32( Pos, Out._rsvd_2_, bLittleEndian );
		std::advance( Pos, 4 );

		Binary::DeserializeUInt32( Pos, Out._rsvd_3_, bLittleEndian );
		std::advance( Pos, 4 );

		Binary::DeserializeUInt32( Pos, Out._rsvd_4_, bLittleEndian );
		std::advance( Pos, 4 );

		// Clamp boolean byte to [0,1]
		if( Out.bAllowTransparency != 0 )
		{
			Out.bAllowTransparency = 1;
		}

		return true;
	}


	template<>
	std::shared_ptr< Asset > AssetLoader::Load< MaterialAsset >( const AssetPath& inPath, std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
	{
		// File Structure
		// [8-byte validation/BOM mark]
		// [32-byte header]
		// Lines.....
		// EOF

		// Each line has the following structure
		// [1-byte TYPE][1-byte RESERVED][1-byte RESERVED][1-byte RESERVED][2-byte KEY_LENGTH][2-byte VALUE_LENGTH][x-bytes KEY][x-bytes VALUE]
		// So first, lets validate the length
		// Each material needs at least
		// Validation Mark, Header, 1 line (minimum 8 bytes).. total = 8 + 32 + 8 -> 48 bytes
		if( std::distance( Begin, End ) < 48 )
		{
			Console::WriteLine( "[ERROR] Material Loader: Failed to load material asset '", inPath.GetPath(), "' because there wasnt enough data in the file" );
			return nullptr;
		}

		auto Pos = Begin;

		// First, read in validation mark, check endian order & validate the file
		uint64 validateMark;
		Binary::DeserializeUInt64( Pos, validateMark, false );
		std::advance( Pos, 8 );

		bool bLittleEndian = false;
		if( validateMark == 0x69'AF'FF'69'12'34'00'01 )
		{
			// Passed validation, no byte flip needed
		}
		else if( validateMark == 0x01'00'34'12'69'FF'AF'69 )
		{
			// Passed validation, but byte flip is needed
			bLittleEndian = true;
		}
		else
		{
			Console::WriteLine( "[ERROR] Material Loader: Failed to load material asset '", inPath.GetPath(), "' because the file was invalid [Validation Failed]" );
			return nullptr;
		}

		// Next, we need to read in the 32-byte header
		MaterialFile Data;
		ReadMaterialHeader( Pos, bLittleEndian, Data.Header );

		// Lets check to see if the size of the size of the file matches with what the header is telling us
		uint32 minSize = 40 + ( Data.Header.EntryCount * 8 );
		if( std::distance( Begin, End ) < minSize )
		{
			Console::WriteLine( "[ERROR] Material Loader: Failed to load material asset '", inPath.GetPath(), "' because the data in the header was invalid (Line Count Invalid)" );
			return nullptr;
		}

		// Now, we need to loop through and read the lines until there is no more data left
		// Each line has a minimum size of 8-bytes, so we will loop untli there is less than 8-bytes left in the buffer
		while( std::distance( Pos, End ) >= 8 )
		{
			// Line Structure: [1-byte TYPE][1-byte RSVD][1-byte RSVD][1-byte RSVD][2-byte KEY SIZE][2-byte VAL SIZE][X-bytes KEY STRING][X-bytes VAL DATA]
			uint8 valueType;
			Binary::DeserializeUInt8( Pos, valueType );
			std::advance( Pos, 1 );

			uint8 _rsvd_1_;
			Binary::DeserializeUInt8( Pos, _rsvd_1_ );
			std::advance( Pos, 1 );

			uint8 _rsvd_2_;
			Binary::DeserializeUInt8( Pos, _rsvd_2_ );
			std::advance( Pos, 1 );

			uint8 _rsvd_3_;
			Binary::DeserializeUInt8( Pos, _rsvd_3_ );
			std::advance( Pos, 1 );

			uint16 keySize;
			Binary::DeserializeUInt16( Pos, keySize, bLittleEndian );
			std::advance( Pos, 2 );

			uint16 valSize;
			Binary::DeserializeUInt16( Pos, valSize, bLittleEndian );
			std::advance( Pos, 2 );

			// Validate what we have read so far..

		}
	}
}
