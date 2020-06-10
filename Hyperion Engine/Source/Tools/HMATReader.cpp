/*==================================================================================================
	Hyperion Engine
	Source/Tools/HMATReader.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Tools/HMATReader.h"
#include "Hyperion/Library/Binary.h"


namespace Hyperion
{

	HMATReader::HMATReader( DataReader& inReader )
		: m_Reader( inReader ), m_bInvalidFormat( true ), m_Size( (uint32)inReader.Size() )
	{
		_ReadFormat();
	}


	HMATReader::~HMATReader()
	{

	}


	void HMATReader::_ReadFormat()
	{
		static const std::vector< byte > correctSequence =
		{
			0x1A, 0xA1, 0xFF, 0x28,
			0x9D, 0xD9, 0x00, 0x04
		};

		// First, check if we meet the minimum data requirment
		if( m_Reader.Size() >= 16 )
		{
			m_Reader.SeekBegin();
			std::vector< byte > headerData;

			if( m_Reader.ReadBytes( headerData, 16 ) == DataReader::ReadResult::Success ||
				headerData.size() != 8 )
			{
				if( std::equal( correctSequence.begin(), correctSequence.end(), headerData.begin() ) )
				{
					m_bInvalidFormat = false;
				}
			}
		}
	}


	HMATReader::Result HMATReader::GetEntryCount( uint16& outCount )
	{
		if( m_bInvalidFormat )
		{
			return Result::InvalidFormat;
		}

		std::vector< byte > countData;
		m_Reader.SeekBegin( 8 );

		if( m_Reader.ReadBytes( countData, 2 ) != DataReader::ReadResult::Success ||
			countData.size() != 2 )
		{
			return Result::ReadFailed;
		}

		Binary::DeserializeUInt16( countData.begin(), outCount );
		return Result::Success;
	}
	

	void HMATReader::Begin()
	{
		if( !m_bInvalidFormat )
		{
			m_Reader.SeekBegin( 16 );
		}
	}


	bool HMATReader::Next()
	{
		if( m_bInvalidFormat ) { return false; }

		auto offset = m_Reader.GetOffset();
		if( offset + 8ll > m_Size ) { return false; }

		return true;
	}


	HMATReader::Result HMATReader::ReadEntry( String& outKey, std::any& outValue )
	{
		if( m_bInvalidFormat ) { return Result::InvalidFormat; }

		// Read the non-dynamic part of the entry
		std::vector< byte > nonStaticData;
		if( m_Reader.ReadBytes( nonStaticData, 8 ) != DataReader::ReadResult::Success ||
			nonStaticData.size() != 8 )
		{
			return Result::ReadFailed;
		}

		auto It = nonStaticData.begin();
		uint16 keyLen;
		uint16 valLen;
		uint8 valType;

		Binary::DeserializeUInt16( It, keyLen );
		std::advance( It, 2 );

		Binary::DeserializeUInt8( It, valType );
		std::advance( It, 1 );

		Binary::DeserializeUInt16( It, valLen );

		// Validate the value type
		if( valType > (uint32)ValueType::Texture )
		{
			return Result::InvalidValue;
		}

		// Next, read the rest of the data for this entry
		auto keyValueSize = keyLen + valLen;
		std::vector< byte > keyValueData;

		if( m_Reader.ReadBytes( keyValueData, keyValueSize ) != DataReader::ReadResult::Success ||
			keyValueData.size() != keyValueSize )
		{
			return Result::ReadFailed;
		}

		// The key is a utf-16 string, big endian
		std::vector< byte > keyData( keyValueData.begin(), keyValueData.begin() + keyLen );
		std::vector< byte > valueData( keyValueData.begin() + keyLen, keyValueData.end() );

		std::vector< byte >().swap( keyValueData );

		outKey = String( keyData, StringEncoding::UTF16 );
		if( outKey.IsWhitespaceOrEmpty() )
		{
			return Result::InvalidKey;
		}

		switch( static_cast< ValueType >( valType ) )
		{
		case ValueType::Boolean:

			if( valueData.size() != 1 ) { return Result::InvalidValue; }
			outValue = ( valueData.at( 0 ) != 0 );
			break;

		case ValueType::Int32:

			if( valueData.size() != 4 ) { return Result::InvalidValue; }
			int32 ival;
			Binary::DeserializeInt32( valueData.begin(), ival );
			outValue = ival;
			break;

		case ValueType::UInt32:

			if( valueData.size() != 4 ) { return Result::InvalidValue; }
			uint32 uval;
			Binary::DeserializeUInt32( valueData.begin(), uval );
			outValue = uval;
			break;

		case ValueType::Float:

			if( valueData.size() != 4 ) { return Result::InvalidValue; }
			float fval;
			Binary::DeserializeFloat( valueData.begin(), fval );
			outValue = fval;
			break;

		case ValueType::Texture:

			if( valueData.size() != 4 ) { return Result::InvalidValue; }
			TextureReference tval;
			Binary::DeserializeUInt32( valueData.begin(), tval.Identifier );
			outValue = tval;
			break;

		case ValueType::String:
		default:

			outValue = String( valueData, StringEncoding::UTF16 ).ToLower();
			break;
		}

		return Result::Success;
	}


}