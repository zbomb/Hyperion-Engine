/*==================================================================================================
	Hyperion Engine
	Source/Tools/HMATWriter.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Tools/HMATWriter.h"
#include "Hyperion/Assets/TextureAsset.h"



namespace Hyperion
{

	HMATWriter::HMATWriter( IFile& inFile )
		: m_File( inFile )
	{
		if( !inFile.CanWriteStream() )
		{
			Console::WriteLine( "[ERROR] HMATWriter: Failed to construct, the target file doesnt allow writing!" );
		}
	}


	HMATWriter::~HMATWriter()
	{

	}


	bool HMATWriter::_CheckKey( const String& inKey )
	{
		if( m_Values.find( inKey ) != m_Values.end() )
		{
			Console::WriteLine( "[DEBUG] HMATWriter: Failed to add '", inKey, "' because there is already an entry using this key!" );
			return false;
		}

		return true;
	}


	bool HMATWriter::AddEntry( const String& inKey, bool inValue, bool bOverride /* = false */ )
	{
		// Convert all keys to lowercase, and ensure a value doesnt already exist with this key
		auto lowerKey = inKey.ToLower();
		if( !bOverride && !_CheckKey( lowerKey ) ) { return false; }

		m_Values[ lowerKey ] = inValue;
		return true;
	}


	bool HMATWriter::AddEntry( const String& inKey, int32 inValue, bool bOverride /* = false */ )
	{
		auto lowerKey = inKey.ToLower();
		if( !bOverride && !_CheckKey( lowerKey ) ) { return false; }
		
		m_Values[ lowerKey ] = inValue;
		return true;
	}


	bool HMATWriter::AddEntry( const String& inKey, uint32 inValue, bool bOverride /* = false */ )
	{
		auto lowerKey = inKey.ToLower();
		if( !bOverride && !_CheckKey( lowerKey ) ) { return false; }
		
		m_Values[ lowerKey ] = inValue;
		return true;
	}


	bool HMATWriter::AddEntry( const String& inKey, float inValue, bool bOverride /* = false */ )
	{
		auto lowerKey = inKey.ToLower();
		if( !bOverride && !_CheckKey( lowerKey ) ) { return false; }
		
		m_Values[ lowerKey ] = inValue;
		return true;
	}


	bool HMATWriter::AddEntry( const String& inKey, const String& inValue, bool bOverride /* = false */ )
	{
		auto lowerKey = inKey.ToLower();
		if( !bOverride && !_CheckKey( lowerKey ) ) { return false; }
		
		m_Values[ lowerKey ] = inValue;
		return true;
	}


	bool HMATWriter::AddEntry( const String& inKey, const std::shared_ptr< TextureAsset >& inTexture, bool bOverride /* = false */ )
	{
		auto lowerKey = inKey.ToLower();
		if( !bOverride && !_CheckKey( lowerKey ) ) { return false; }
		
		TextureWrapper id;
		id.value = 0;

		if( inTexture )
		{
			id.value = inTexture->GetIdentifier();
		}

		m_Values[ lowerKey ] = id;
		return true;
	}


	HMATWriter::Result HMATWriter::Flush()
	{
		if( !m_File.CanWriteStream() ) { return Result::InvalidFile; }

		// Were first going to serialize all data to a vector before writing, so we can write in a single call
		std::vector< byte > fileData;

		// First, we start out with the header data
		static const std::vector< byte > correctSequence =
		{
			0x1A, 0xA1, 0xFF, 0x28,
			0x9D, 0xD9, 0x00, 0x04
		};

		fileData.insert( fileData.end(), correctSequence.begin(), correctSequence.end() );

		// Next, we need to push a 2 byte unsigned int, thats the entry count
		if( m_Values.size() > std::numeric_limits< uint16 >::max() )
		{
			return Result::ValueOverflow;
		}

		Binary::SerializeUInt16( (uint16) m_Values.size(), fileData );

		// Next, we have 6 bytes of space reserved for future use
		fileData.insert( fileData.end(), 6, 0 );

		// Now we start writing each value in the list
		for( auto& entry : m_Values )
		{
			// First, we write a 2-byte number for the length of the key in bytes
			std::vector< byte > keyData;
			if( !entry.first.CopyData( keyData, StringEncoding::UTF16 ) ||
				keyData.size() > std::numeric_limits< uint16 >::max() ||
				!entry.second.has_value() )
			{
				return Result::InvalidValue;
			}

			Binary::SerializeUInt16( (uint16) keyData.size(), fileData );

			// Now we want to get the data for the value
			uint8 valType	= 0;
			std::vector< byte > valData;

			auto& type = entry.second.type();
			if(	type == typeid( bool ) )
			{ 
				valType		= (uint8) ValueType::Boolean; 

				valData.push_back( std::any_cast< bool >( entry.second ) ? 1 : 0 );
			}
			else if( type == typeid( int32 ) )
			{ 
				valType		= (uint8) ValueType::Int32; 

				int32 val = std::any_cast< int32 >( entry.second );
				Binary::SerializeInt32( val, valData );
			}
			else if( type == typeid( uint32 ) )
			{ 
				valType		= (uint8) ValueType::UInt32; 

				uint32 val = std::any_cast< uint32 >( entry.second );
				Binary::SerializeUInt32( val, valData );
			}
			else if( type == typeid( float ) )
			{ 
				valType		= (uint8) ValueType::Float;
				
				float val = std::any_cast< float >( entry.second );
				Binary::SerializeFloat( val, valData );
			}
			else if( type == typeid( String ) )
			{ 
				valType		= (uint8) ValueType::String; 
				
				String val = std::any_cast< String >( entry.second );
				if( !val.CopyData( valData, StringEncoding::UTF16 ) ) 
				{
					Console::WriteLine( "[ERROR] HMATWriter: Failed to copy string to output!" );
					return Result::InvalidValue;
				}
			}
			else if( type == typeid( TextureWrapper ) )
			{ 
				valType		= (uint8) ValueType::Texture; 
				
				TextureWrapper val = std::any_cast< TextureWrapper >( entry.second );
				Binary::SerializeUInt32( val.value, valData );
			}
			else { return Result::InvalidValue; }

			// Nex thing were going to write to the output, is the value type
			Binary::SerializeUInt8( valType, fileData );

			// Now we need to write the value size in bytes
			if( valData.size() > std::numeric_limits< uint16 >::max() )
			{
				Console::WriteLine( "[ERROR] HMATWriter: Value data is larger than the maximum size!" );
				return Result::InvalidValue;
			}

			Binary::SerializeUInt16( (uint16) valData.size(), fileData );

			// Next, we have 3-bytes of reserved data
			fileData.insert( fileData.end(), 3, 0 );

			// Now, we have to write the key data
			fileData.insert( fileData.end(), keyData.begin(), keyData.end() );

			// Finally, write the value data
			fileData.insert( fileData.end(), valData.begin(), valData.end() );
		}

		// Now that the data is built, we have to write it to file
		DataWriter writer( m_File );
		if( !writer.WriteBytes( fileData ) )
		{
			return Result::WriteFailed;
		}

		return Result::Success;
	}

}