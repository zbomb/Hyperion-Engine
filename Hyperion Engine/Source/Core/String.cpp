/*==================================================================================================
	Hyperion Engine
	Source/Core/String.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include <iostream>
#include <algorithm>
#include <codecvt>

#include "Hyperion/Core/String.h"
#include "Hyperion/Library/UTF8.hpp"
#include "Hyperion/Library/UTF16.hpp"
#include "Hyperion/Library/Math/MathCore.h"
#include "Hyperion/Library/Binary.h"



namespace Hyperion
{
	/*
		Static Variable Definitions
	*/
	std::unordered_map< std::string, std::shared_ptr< Localization::Cache::Instance > > Localization::Cache::m_Values;
	std::map< LanguageID, LanguageInfo > Localization::m_Languages;
	LanguageID Localization::m_LastLangID( LANG_NONE );
	LanguageInfo Localization::Language_None{ { 0x44, 0x65, 0x66, 0x61, 0x75, 0x6C, 0x74 }, "none", LANG_NONE };



	/*=========================================================================================================
		Localization Cache
	=========================================================================================================*/

	/*
		Localization::Cache::CreateInstance
	*/
	std::shared_ptr< Localization::Cache::Instance > Localization::Cache::CreateInstance( const std::string& inStr )
	{
		// If we try and create a null string, were just going to return nullptr instead
		if( inStr.size() <= 0 )
			return nullptr;

		// Check if this string already exists in the cache
		auto Entry = m_Values.find( inStr );
		if( Entry == m_Values.end() )
		{
			// Create empty string instance.. this way.. if we load this string later on, it will update
			auto newInst				= std::make_shared< Instance >();
			newInst->m_Data				= nullptr;
			newInst->m_RefCount			= 1;
			newInst->m_LocalizationKey	= inStr;

			auto newEntry = m_Values.emplace( inStr, newInst );
			return newEntry.first->second;
		}
		else
		{
			if( Entry->second )
			{
				Entry->second->m_RefCount++;
				return Entry->second;
			}
			else
			{
				Console::WriteLine( "[ERROR] Localized String Cache: Found invalid Instance in cache... returning null!" );
				return nullptr;
			}
		}
	}

	/*
		Localization::Cache::DestroyInstance
	*/
	void Localization::Cache::DestroyInstance( std::shared_ptr< Localization::Cache::Instance >& In )
	{
		if( !In || In->m_LocalizationKey.size() <= 0 )
			return;

		auto Entry = m_Values.find( In->m_LocalizationKey );
		if( Entry == m_Values.end() )
		{
			Console::WriteLine( "[ERROR] Localized String Cache: Couldnt find string data in the cache!" );
					
			// Still, decrement ref counter
			if( In->m_RefCount > 0 )
				In->m_RefCount--;
		}
		else
		{
			if( Entry->second )
			{
				if( Entry->second->m_RefCount <= 1 )
				{
					// Only erase if there is no value attached
					if( !Entry->second->m_Data )
					{
						m_Values.erase( Entry );
					}
				}
				else
				{
					Entry->second->m_RefCount--;
				}
			}
			else
			{
				Console::WriteLine( "[ERROR] Localized String Cache: Bad string instance found in cache... removing..." );
				m_Values.erase( Entry );
			}
		}

		In.reset();
	}

	/*=========================================================================================================
		Localization Library
	=========================================================================================================*/

	/*
		Localization::GetLanguage
	*/
	const LanguageInfo& Localization::GetLanguage( LanguageID Identifier )
	{
		if( Identifier != LANG_NONE )
		{
			for( auto It = m_Languages.begin(); It != m_Languages.end(); It++ )
			{
				if( It->first == Identifier )
				{
					return It->second;
				}
			}
		}

		return Language_None;
	}

	/*
		Localization::GetLanguage
	*/
	const LanguageInfo& Localization::GetLanguage( const std::string& Key )
	{
		if( Key.size() > 0 )
		{
			std::string lowerKey;
			lowerKey.resize( Key.size() );
			std::transform( Key.begin(), Key.end(), lowerKey.begin(), 
							[] ( unsigned char c ) -> unsigned char { return std::tolower( c ); } );

			for( auto It = m_Languages.begin(); It != m_Languages.end(); It++ )
			{
				if( It->second.Key == lowerKey )
				{
					return It->second;
				}
			}
		}

		return Language_None;
	}

	/*
		Localization::LanguageExists
	*/
	bool Localization::LanguageExists( LanguageID Identifier )
	{
		return GetLanguage( Identifier ).Identifier != LANG_NONE;
	}

	/*
		Localization::LanguageExists
	*/
	bool Localization::LanguageExists( const std::string& Key )
	{
		return GetLanguage( Key ).Identifier != LANG_NONE;
	}

	/*
		Localization::AddLanguage
	*/
	LanguageID Localization::AddLanguage( LanguageInfo& In )
	{
		if( In.Key.size() <= 0  )
		{
			// Invalid Language
			Console::WriteLine( "[ERROR] Localization Dictionary: Attempt to register invalid language!" );
			return LANG_NONE;
		}

		// Convert key to lowercase
		std::transform( In.Key.begin(), In.Key.end(), In.Key.begin(),
						[] ( unsigned char c ) -> unsigned char { return std::tolower( c ); } );

		// Check if this key already exists
		if( LanguageExists( In.Key ) )
		{
			Console::WriteLine( "[ERROR] Localization Dictionary: Attempt to register language with existing name!" );
			return LANG_NONE;
		}

		// Generate Unique id
		In.Identifier = ++m_LastLangID;

		// Add to map
		m_Languages[ In.Identifier ] = In;
		return In.Identifier;
	}

	/*
		Localization::CreateOrUpdateDefinition
	*/
	bool Localization::CreateOrUpdateDefinition( LanguageID Lang, const std::string& Key, std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
	{
		// Validate this language (LANG_NONE) is allowed
		if( Lang != LANG_NONE && !LanguageExists( Lang ) )
		{
			Console::WriteLine( "[ERROR] Localization Dictionary: Attempt to create/update definition, but invalid language specified!" );
			return false;
		}

		// Ensure key is valid
		if( Key.size() <= 0 )
		{
			Console::WriteLine( "[ERROR] Localization Dictionary: Attempt to create/update definition, but the key is empty!" );
			return false;
		}

		// Convert key to lowercase
		std::string lowerKey;
		lowerKey.resize( Key.size() );
		std::transform( Key.begin(), Key.end(), lowerKey.begin(),
						[] ( unsigned char c ) -> unsigned char { return std::tolower( c ); } );

		// Create/Update the actual value in the cache
		std::shared_ptr< Cache::Instance >& Entry = Cache::m_Values[ Key ];
		if( !Entry )
		{
			Entry = std::make_shared< Cache::Instance >();
		}

		Entry->m_Lang				= Lang;
		Entry->m_LocalizationKey	= Key;
		Entry->m_Data				= std::make_shared< const std::vector< byte > >( Begin, End );
			
		return true;
	}

	/*
		Localization::LookupDefinition
	*/
	std::shared_ptr< Localization::Cache::Instance > Localization::LookupDefinition( const std::string& Key )
	{
		// Validate key
		if( Key.size() <= 0 )
		{
			Console::WriteLine( "[ERROR] Localization Dictionary: Attempt to lookup definition, with a null key!" );
			return nullptr;
		}

		// Ensure key is lowercase
		std::string lowerKey;
		lowerKey.resize( Key.size() );
		std::transform( Key.begin(), Key.end(), lowerKey.begin(), 
						[] ( unsigned char c ) -> unsigned char { return std::tolower( c ); } );

		// Check if this is in the dictionary
		auto Entry = Cache::m_Values.find( lowerKey );
		if( Entry == Cache::m_Values.end() || !Entry->second )
		{
			return nullptr;
		}

		return Entry->second;
	}

	/*
		Localization::ClearLanguage
	*/
	void Localization::ClearLanguage( LanguageID Lang )
	{
		for( auto It = Cache::m_Values.begin(); It != Cache::m_Values.end(); )
		{
			if( !It->second || It->second->m_Lang == Lang )
			{
				// Check ref count.. if its zero, then we can erase it.. otherwise were going to just clear the value
				if( It->second->m_RefCount > 0 )
				{
					It->second->m_Data.reset();
					It++;
				}
				else
				{
					It = Cache::m_Values.erase( It );
				}
			}
		}
	}

	/*
		Localization::ClearDefinition
	*/
	bool Localization::ClearDefinition( const std::string& Key )
	{
		if( Key.size() <= 0 )
		{
			Console::WriteLine( "[ERROR] Localization Dictionary: Attempt to clear definition, with a null key!" );
			return false;
		}

		// Convert key to lowercase
		std::string lowerKey;
		lowerKey.resize( Key.size() );
		std::transform( Key.begin(), Key.end(), lowerKey.begin(),
						[] ( unsigned char c ) -> unsigned char { return std::tolower( c ); } );

		// Clear the data ptr from this value in the cache, if theres no refs.. then we can erase it
		auto Entry = Cache::m_Values.find( lowerKey );
		if( Entry == Cache::m_Values.end() )
		{
			return false;
		}

		if( Entry->second->m_RefCount > 0 )
		{
			Entry->second->m_Data.reset();
		}
		else
		{
			Cache::m_Values.erase( Entry );
		}

		return true;
	}

	/*
		Localization::DebugCache
	*/
	void Localization::DebugCache()
	{
		Console::Write( "\n============================== LOCALIZATION CACHE BEGIN ==============================\n",
						"-------> Definition Count: ", Cache::m_Values.size(), "\n",
						"-------> Language Count: ", m_Languages.size(), "\n",
						"-------> Language List: \n\n" );


		for( auto It = m_Languages.begin(); It != m_Languages.end(); It++ )
		{
			Console::Write( "\t---> Language \"", It->second.Key, "\"\n\t\tIdentifier: ", It->second.Identifier, "\n" );
		}

		Console::Write( "\n------> Definition List:\n\n" );
			
		for( auto It = Cache::m_Values.begin(); It != Cache::m_Values.end(); It++ )
		{
			if( !It->second )
			{
				Console::Write( "\t---> Null Definition\n",
								"\t\tKey: ", It->first, "\n" );
			}
			else
			{
				Console::Write( "\t---> Definition\n",
								"\t\tKey: ", It->second->m_LocalizationKey, "\n",
								"\t\tLangID: ", It->second->m_Lang, "\n" );

				if( It->second->m_Data )
				{
					Console::Write( "\t\tValue: ", String::GetSTLString( *It->second->m_Data ), "\n\n" );
				}
				else
				{
					Console::Write( "\t\tValue: NULL\n\n" );
				}
			}
		}

		Console::Write( "==================================== LOCALIZATION CACHE END ====================================\n" );
	}

	/*=========================================================================================================
		Non Cached String Data
	=========================================================================================================*/
	String::NonCachedStringData::NonCachedStringData()
		: m_Data( nullptr )
	{}

	String::NonCachedStringData::NonCachedStringData( const NonCachedStringData& Other )
		: m_Data( Other.m_Data )
	{}

	String::NonCachedStringData::NonCachedStringData( const std::vector< byte >& inData, StringEncoding Encoding, bool bKnownByteOrder /* = false */, bool bLittleEndian /* = false */ )
		: NonCachedStringData( ChangeEncoding_Impl( inData, Encoding, StringEncoding::UTF8, bKnownByteOrder, bLittleEndian ) )
	{}

	String::NonCachedStringData::NonCachedStringData( const std::vector< byte >& inData )
		: m_Data( std::make_shared< const std::vector< byte > >( inData ) )
	{}

	String::NonCachedStringData::~NonCachedStringData()
	{
		m_Data.reset();
	}

	void String::NonCachedStringData::operator=( const NonCachedStringData& Other )
	{
		if( m_Data != Other.m_Data )
		{
			// Simple assignment
			m_Data = Other.m_Data;
		}
	}

#if _DEBUG

	void String::NonCachedStringData::UpdateDebugPointers( byte** ppBegin, byte** ppEnd )
	{
		if( ppBegin != nullptr && ppEnd != nullptr )
		{
			if( m_Data )
			{
				*ppBegin = const_cast<byte*>( m_Data->data() );
				*ppEnd = *ppBegin + m_Data->size();
			}
			else
			{
				*ppBegin = nullptr;
				*ppEnd = nullptr;
			}
		}
	}

#endif


	/*=========================================================================================================
		Localized String Data
	=========================================================================================================*/

	/*
		String::LocalizedStringData::GetData
	*/
	std::shared_ptr< const std::vector< byte > > String::LocalizedStringData::GetData()
	{
		if( m_LocalizedRef && m_LocalizedRef->m_Data )
		{
			// Ensure its valid
			return m_LocalizedRef->m_Data;
		}
		else if( m_NonLocalizedRef )
		{
			// Default string
			return m_NonLocalizedRef;
		}

		// If everything fails, return null
		return nullptr;
	}

	/*
		String::LocalizedStringData::LocalizedStringData
	*/
	String::LocalizedStringData::LocalizedStringData()
	{}

	/*
		String::LocalizedStringData::LocalizedStringData
	*/
	String::LocalizedStringData::LocalizedStringData( const LocalizedStringData& Other )
	{
		if( Other.m_LocalizedRef )
		{
			Other.m_LocalizedRef->m_RefCount++;
			m_LocalizedRef = Other.m_LocalizedRef;
		}

		if( Other.m_NonLocalizedRef )
		{
			m_NonLocalizedRef = Other.m_NonLocalizedRef;
		}
	}

	/*
		String::LocalizedStringData::LocalizedStringData
	*/
	String::LocalizedStringData::LocalizedStringData( const std::string& inKey )
		: m_LocalizedRef( Localization::Cache::CreateInstance( inKey ) ), 
		m_NonLocalizedRef( nullptr )
	{}

	/*
		String::LocalizedStringData::LocalizedStringData
	*/
	String::LocalizedStringData::LocalizedStringData( const std::string& inKey, const std::vector< byte >& inDefault, StringEncoding Enc /*= StringEncoding::ASCII*/ )
		: m_LocalizedRef( Localization::Cache::CreateInstance( inKey ) ),
		m_NonLocalizedRef( std::make_shared< const std::vector< byte > >( ChangeEncoding_Impl( inDefault, Enc, StringEncoding::UTF8 ) ) )
	{}

	/*
		String::LocalizedStringData::LocalizedStringData
	*/
	String::LocalizedStringData::LocalizedStringData( const std::string& inKey, const std::string& inDefault, StringEncoding Enc /*= StringEncoding::ASCII*/ )
		: LocalizedStringData( inKey, std::vector< byte >( inDefault.begin(), inDefault.end() ), Enc )
	{}

	/*
		String::LocalizedStringData::LocalizedStringData
	*/
	String::LocalizedStringData::LocalizedStringData( const std::string& inKey, const char* inDefault, StringEncoding Enc /*= StringEncoding::ASCII*/ )
		: LocalizedStringData( inKey, std::string( inDefault ), Enc )
	{}

	/*
		String::LocalizedStringData::LocalizedStringData
	*/
	String::LocalizedStringData::LocalizedStringData( const char* inKey )
		: LocalizedStringData( std::string( inKey ) )
	{}

	/*
		String::LocalizedStringData::LocalizedStringData
	*/
	String::LocalizedStringData::LocalizedStringData( const char* inKey, const std::vector< byte >& inDefault, StringEncoding Enc /*= StringEncoding::ASCII*/ )
		: LocalizedStringData( std::string( inKey ), inDefault, Enc )
	{}

	/*
		String::LocalizedStringData::LocalizedStringData
	*/
	String::LocalizedStringData::LocalizedStringData( const char* inKey, const std::string& inDefault, StringEncoding Enc /*= StringEncoding::ASCII*/ )
		: LocalizedStringData( std::string( inKey ), std::vector< byte >( inDefault.begin(), inDefault.end() ), Enc )
	{}

	/*
		String::LocalizedStringData::LocalizedStringData
	*/
	String::LocalizedStringData::LocalizedStringData( const char* inKey, const char* inDefault, StringEncoding Enc /*= StringEncoding::ASCII*/ )
		: LocalizedStringData( std::string( inKey ), std::string( inDefault ), Enc )
	{}

	/*
		String::LocalizedStringData::~LocalizedStringData
	*/
	String::LocalizedStringData::~LocalizedStringData()
	{
		if( m_LocalizedRef )
		{
			Localization::Cache::DestroyInstance( m_LocalizedRef );
			m_LocalizedRef.reset();
		}

		if( m_NonLocalizedRef )
		{
			m_NonLocalizedRef.reset();
		}
	}

	/*
		String::LocalizedStringData::operator=
	*/
	void String::LocalizedStringData::operator=( const LocalizedStringData& Other )
	{
		if( m_LocalizedRef != Other.m_LocalizedRef )
		{
			if( m_LocalizedRef )
			{
				Localization::Cache::DestroyInstance( m_LocalizedRef );
				m_LocalizedRef.reset();
			}

			if( Other.m_LocalizedRef )
			{
				Other.m_LocalizedRef->m_RefCount++;
				m_LocalizedRef = Other.m_LocalizedRef;
			}

		}

		if( m_NonLocalizedRef != Other.m_NonLocalizedRef )
		{
			if( m_NonLocalizedRef )
			{
				m_NonLocalizedRef.reset();
			}

			if( Other.m_NonLocalizedRef )
			{
				m_NonLocalizedRef = Other.m_NonLocalizedRef;
			}
		}
	}

	#if _DEBUG

	void String::LocalizedStringData::UpdateDebugPointers( byte** ppBegin, byte** ppEnd )
	{
		if( ppBegin != nullptr && ppEnd != nullptr )
		{
			auto data = this->GetData();
			if( data )
			{
				*ppBegin = const_cast<byte*>( data->data() );
				*ppEnd = *ppBegin + data->size();
			}
			else
			{
				*ppBegin = nullptr;
				*ppEnd = nullptr;
			}
		}
	}

	#endif


	/*=========================================================================================================
		String Objects
	=========================================================================================================*/

	#if _DEBUG

	void String::_DebugSetup()
	{
		if( m_Data )
		{
			m_Data->UpdateDebugPointers( &_StrBegin, &_StrEnd );
		}
		else
		{
			_StrBegin = nullptr;
			_StrEnd = nullptr;
		}
	}

	#endif

	/*
		String::String
	*/
	String::String( const std::shared_ptr< IStringData >& inRawData )
		: m_Data( inRawData )
	{
		#if _DEBUG
		_DebugSetup();
		#endif
	}

	/*
		String::String
	*/
	String::String( const std::vector< byte >& inData, StringEncoding Enc /*= StringEncoding::ASCII */ )
		: m_Data( std::dynamic_pointer_cast< IStringData >( std::make_shared< NonCachedStringData >( inData, Enc ) ) )
	{
		#if _DEBUG
		_DebugSetup();
		#endif
	}

	/*
		String::String
	*/
	String::String( const std::string& inStr, StringEncoding Enc /*= StringEncoding::ASCII*/ )
		: String( std::vector< byte >( inStr.begin(), inStr.end() ), Enc )
	{
		#if _DEBUG
		_DebugSetup();
		#endif
	}

	/*
		String::String
	*/
	String::String( const char* inStr, StringEncoding Enc /*= StringEncoding::ASCII*/ )
		: String( std::string( inStr ), Enc )
	{
		#if _DEBUG
		_DebugSetup();
		#endif
	}

	/*
		String::String
	*/
	String::String()
		: m_Data( nullptr )
	{
		#if _DEBUG
		_DebugSetup();
		#endif
	}

	/*
		String::String
	*/
	String::String( nullptr_t )
		: m_Data( nullptr )
	{
		#if _DEBUG
		_DebugSetup();
		#endif
	}

	/*
		String::String
	*/
	String::String( const String& Other )
		: m_Data( Other.m_Data )
	{
		#if _DEBUG
		_DebugSetup();
		#endif
	}

	/*
		String::String
	*/
	String::String( const iterator& Begin, const iterator& End )
	{
		// Ensure theyre from the same source string, and not null
		if( Begin.m_Target != End.m_Target || !Begin.m_Target )
			return;

		// Get the underlying char start position in the data vector, copy into the string data and were done!
		m_Data =  std::dynamic_pointer_cast< IStringData >( std::make_shared< NonCachedStringData >( std::vector< byte >( Begin.m_Iter, End.m_Iter ) ) );

		#if _DEBUG
		_DebugSetup();
		#endif
	}

	/*
		String::String
	*/
	String::String( const std::wstring& inStr, StringEncoding Enc )
	{
		// This is a little complicated due to the nature of wchar_t, we dont really know the size of a wchar_t, 
		// the encoding of the source data (caller needs to specify!) or the byte order
		// We can determine the size of whcar_t and the byte order.. so we can build the string with a byte buffer and the correct encoding
		auto isLittleEndian = Binary::IsLittleEndian();
		auto wcharSize = sizeof( wchar_t );
		std::vector< byte > sortedData;
		sortedData.reserve( inStr.size() * wcharSize );

		// We need to reinterpret the wstring data as a byte array
		const byte* strData = reinterpret_cast< const byte* >( inStr.data() );

		// Iterate through each character, and its position in the byte array is..
		// strData + [Index] * sizeof( wchar_t )
		for( size_t ich = 0; ich < inStr.size(); ich++ )
		{
			// Get pointer to where this characters data begins
			const byte* charStart = strData + ( ich * wcharSize );

			// Now, we need to take endian order into account before copying this into our buffer
			for( unsigned int ib = 0; ib < wcharSize; ib++ )
			{
				if( isLittleEndian )
				{
					// Copy each character byte into the buffer, in reverse order
					sortedData.push_back( *( charStart + ( wcharSize - 1 ) - ib ) );
				}
				else
				{
					// Copy each character byte into the buffer in order
					sortedData.push_back( *( charStart + ib ) );
				}
			}
		}

		// Now, were going to construct the NonLocalizedStringData, but were going to specify the byte order so
		// the encoding conversion doesnt have to read through the entire string to auto-detect the byte order
		m_Data = std::dynamic_pointer_cast< IStringData >( std::make_shared< NonCachedStringData >( std::move( sortedData ), Enc, true, false ) );

		#if _DEBUG
		_DebugSetup();
		#endif
	}

	/*
		String::String
	*/
	String::String( const wchar_t* inStr, StringEncoding Enc )
		: String( std::wstring( inStr ), Enc )
	{
		#if _DEBUG
		_DebugSetup();
		#endif
	}

	/*
		String::Data
	*/
	std::shared_ptr< const std::vector< byte > > String::Data() const 
	{ 
		return m_Data ? m_Data->GetData() : nullptr; 
	}

	/*
		String::operator=
	*/
	void String::operator=( const String& Other )
	{
		// Basically, were going to just copy the IStringData from the other string in
		m_Data = Other.m_Data;
	}

	void String::operator=( nullptr_t )
	{
		Clear();
	}

	void String::Clear()
	{
		m_Data.reset();
	}

	/*
		String::IsEmpty
	*/
	bool String::IsEmpty() const
	{
		return Data() ? false : true;
	}

	/*
		String::begin
	*/
	String::iterator String::begin() const
	{
		// Create iterator, pointing to the internal data vector, at the begining
		std::shared_ptr< const std::vector< byte > > internalData = m_Data ? m_Data->GetData() : nullptr;
		if( internalData )
		{
			return iterator( internalData, internalData->begin() );
		}
		else
		{
			// Null iterator
			return iterator( internalData );
		}
	}

	/*
		String::end
	*/
	String::iterator String::end() const
	{
		// Create iterator, pointing to the back of the internal data vector
		std::shared_ptr< const std::vector< byte > > internalData = m_Data ? m_Data->GetData() : nullptr;
		if( internalData )
		{
			return iterator( internalData, internalData->end() );
		}
		else
		{
			// Null iterator
			return iterator( internalData );
		}
	}

	std::string String::GetU8Str() const
	{
		std::shared_ptr< const std::vector< byte > > internalData = m_Data ? m_Data->GetData() : nullptr;
		if( internalData )
		{
			return std::string( internalData->begin(), internalData->end() );
		}
		else
		{
			return std::string();
		}
	}

	std::u16string String::GetU16Str( bool bIncludeBOM /* = false */ ) const
	{
		auto strData = Data();
		std::u16string output;

		if( strData )
		{
			// Convert the raw hyperion string data (utf-8) to a u16string using the utf-16 library
			std::vector< uint32 > codePoints;
			if( Encoding::UTF8::BinaryToCodes( *strData, codePoints ) )
			{
				Encoding::UTF16::CodesToU16String( codePoints.begin(), codePoints.end(), output, bIncludeBOM );
			}
		}

		return output;
	}

	bool String::CopyData( std::vector< byte >& Out, StringEncoding Enc /* = StringEncoding::UTF8 */ ) const
	{
		auto strData = Data();
		if( strData )
		{
			// If were using UTF-8 (native) then just copy into the output buffer
			if( Enc == StringEncoding::UTF8 )
			{
				Out.insert( Out.end(), strData->begin(), strData->end() );
				return true;
			}

			std::vector< uint32 > CodePoints;
			if( !Encoding::UTF8::BinaryToCodes( *strData, CodePoints ) )
			{
				return false;
			}

			if( Enc == StringEncoding::UTF16 )
			{
				return Encoding::UTF16::CodesToBinary( CodePoints, Out );
			}
			else if( Enc == StringEncoding::UTF32 )
			{
				return false;
			}
			else
			{
				// ASCII
				for( auto& p : CodePoints )
				{
					if( p < 256 )
						Out.push_back( p );
				}

				// No-Fail
				return true;
			}
		}
		else
		{
			return true;
		}
	}

	/*
		String::ChangeEncoding_Impl
	*/
	const std::vector< byte > String::ChangeEncoding_Impl( const std::vector< byte >& Source, StringEncoding SourceEncoding, StringEncoding TargetEncoding, bool bKnownByteOrder /* = false */, bool bLittleEndian /* = false */ )
	{
		if( SourceEncoding == TargetEncoding )
			return Source;

		std::vector< uint32 > CodePoints;
		std::vector< byte > Output;

		if( SourceEncoding == StringEncoding::ASCII )
		{
			for( auto It = Source.begin(); It != Source.end(); It++ )
				CodePoints.push_back( (uint32) *It );
		}
		else if( SourceEncoding == StringEncoding::UTF8 )
		{
			if( !Encoding::UTF8::BinaryToCodes( Source, CodePoints ) )
			{
				Console::Write( "[ERROR] String Library: UTF-8 Decode failed!\n" );
			}
		}
		else if( SourceEncoding == StringEncoding::UTF16 )
		{
			if( bKnownByteOrder )
			{
				if( !Encoding::UTF16::BinaryToCodes( Source, CodePoints, bLittleEndian ) )
				{
					Console::Write( "[ERROR] String: UTF-16 decode failed!\n" );
				}
			}
			else
			{
				if( !Encoding::UTF16::BinaryToCodes( Source, CodePoints ) )
				{
					Console::Write( "[ERROR] String Library: UTF-16 Decode failed!\n" );
				}
			}
		}
		else
		{
			Console::Write( "[ERROR] String Library: Attempt to decode an unsupported encoding type!\n" );
			return Output;
		}

		// Now, re-encode with the target encoding type
		if( TargetEncoding == StringEncoding::ASCII )
		{
			// With ASCII, we only can store up to value 255
			bool bOverflow = false;

			for( auto It = CodePoints.begin(); It != CodePoints.end(); It++ )
			{
				auto Value = *It;
				if( Value > 255 )
				{
					bOverflow = true;
					continue;
				}

				Output.push_back( (byte) Value );
			}

			if( bOverflow )
			{
				Console::Write( "[ERROR] String Library: Attempt to encode character in ASCII, that overflowed! This character was ignored\n" );
			}
		}
		else if( TargetEncoding == StringEncoding::UTF8 )
		{
			if( !Encoding::UTF8::CodesToBinary( CodePoints, Output ) )
			{
				Console::Write( "[ERROR] String Library: Failed to encode UTF-8 String!\n" );
			}
		}
		else if( TargetEncoding == StringEncoding::UTF16 )
		{
			if( !Encoding::UTF16::CodesToBinary( CodePoints, Output ) )
			{
				Console::Write( "[ERROR] String Library: Failed to encode UTF-16 string!\n" );
			}
		}
		else
		{
			Console::Write( "[ERROR] String Library: Attempt to encode string in a unsupported format!\n" );
			return Output;
		}

		return Output;
	}

	/*
		String::Find_Impl
	*/
	String::iterator String::Find_Impl( const String& InStr, const std::vector< byte >& inSequence )
	{
		// Basically, were just finding one data sequence within another
		auto thisData = InStr.Data();
		if( !thisData )
		{
			return InStr.end();
		}

		auto resIter = std::search( thisData->begin(), thisData->end(), std::begin( inSequence ), std::end( inSequence ) );
		if( resIter == thisData->end() )
		{
			return InStr.end();
		}
			
		// Convert the data iterator into a character iterator
		return iterator( thisData, resIter );
	}

	/*
		String::Find
	*/
	String::iterator String::Find( const String& InStr, const Char& InChar )
	{
		// We want to get the binary representation of this character
		std::vector< byte > charBytes;
		Encoding::UTF8::EncodePoint( InChar, charBytes );

		return Find_Impl( InStr, charBytes );
	}

	/*
		String::Find
	*/
	String::iterator String::Find( const String& Source, const String& Sequence )
	{
		// First, get the raw data from the other string
		auto otherData = Sequence.Data();
		if( !otherData )
		{
			return Source.end();
		}

		return Find_Impl( Source, *otherData );
	}

	/*
		String::Length
	*/
	size_t String::Length( const String& InStr )
	{
		size_t Output = 0;
		for( auto It = InStr.begin(); It != InStr.end(); It++ )
		{
			Output++;
		}

		return Output;
	}

	/*
		String::ByteCount
	*/
	size_t String::ByteCount( const String& InStr )
	{
		auto internalData = InStr.Data();
		if( internalData )
		{
			return internalData->size();
		}

		return 0;
	}

	/*
		String::Append
	*/
	String String::Append( const String& First, const String& Other )
	{
		std::vector< byte > FinalData;
		auto thisData = First.Data();
		if( thisData )
		{
			FinalData.insert( FinalData.end(), thisData->begin(), thisData->end() );
		}

		auto otherData = Other.Data();
		if( otherData )
		{
			FinalData.insert( FinalData.end(), otherData->begin(), otherData->end() );
		}

		return String( FinalData, StringEncoding::UTF8 );
	}

	/*
		String::Prepend
	*/
	String String::Prepend( const String& First, const String& Prefix )
	{
		return Append( Prefix, First );
	}

	/*
		String::SubStr
	*/
	String String::SubStr( const String& Source, size_t Start, size_t Length )
	{
		// This requires a full scan through the string object
		auto StartIter = Source.begin();
			
		// Ensure were not working on a null string
		if( !StartIter.m_Target || Length == 0 )
		{
			return String();
		}

		for( size_t i = 0; i < Start; i++ )
		{
			StartIter++;

			// Check if we hit the end
			if( StartIter.m_Iter == StartIter.m_Target->end() )
				break;
		}

		auto EndIter = StartIter;
		if( !EndIter.m_Target || EndIter.m_Iter == EndIter.m_Target->end() )
		{
			// We hit the end.....
			return String();
		}

		// Advance end iter Length times, or until we hit the end
		for( size_t i = 0; i < Length; i++ )
		{
			EndIter++;

			// Check if we hit the end
			if( EndIter.m_Iter == EndIter.m_Target->end() )
				break;
		}

		// Ensure we have at least a single character
		if( StartIter == EndIter )
		{
			return String();
		}

		// Now, we need to take the data between the two underlying data iterators, and use it to create a new string
		return String( StartIter, EndIter );
	}

	/*
		String::IsEmpty
	*/
	bool String::IsEmpty( const String& Source )
	{
		return Source.IsEmpty();
	}

	/*
		String::StartsWith
	*/
	bool String::StartsWith( const String& Source, const Char& ToCheck )
	{
		std::vector< byte > CharData;
		Encoding::UTF8::EncodePoint( ToCheck, CharData );

		auto sourceData		= Source.Data();
		auto sourceSize		= sourceData ? sourceData->size() : 0;
		auto charSize		= CharData.size();

		if( sourceSize < charSize || sourceSize == 0 ) 
			return false;

		auto charIter = CharData.begin();
		auto strIter = sourceData->begin();

		for( size_t i = 0; i < charSize; i++ )
		{
			if( *charIter != *strIter )
				return false;

			charIter++;
			strIter++;
		}

		return true;
	}

	/*
		String::StartsWith
	*/
	bool String::StartsWith( const String& Source, const String& Pattern )
	{
		auto sourceData		= Source.Data();
		auto patternData	= Pattern.Data();

		auto sourceSize		= sourceData ? sourceData->size() : 0;
		auto patternSize	= patternData ? patternData->size() : 0;

		if( patternSize == 0 )					return true;
		else if( sourceSize == 0 )				return false;
		else if( sourceSize < patternSize )		return false;

		auto sourceIter = sourceData->begin();
		auto patternIter = patternData->begin();

		for( size_t i = 0; i < patternSize; i++ )
		{
			if( *sourceIter != *patternIter )
				return false;

			sourceIter++;
			patternIter++;
		}

		return true;
	}

	/*
		String::EndsWith
	*/
	bool String::EndsWith( const String& Source, const Char& ToCheck )
	{
		// Convert the character into utf8 data
		std::vector< byte > CharData;
		Encoding::UTF8::EncodePoint( ToCheck, CharData );

		auto sourceData		= Source.Data();
		auto sourceSize		= sourceData ? sourceData->size() : 0;
		auto charSize		= CharData.size();

		if( sourceSize < charSize || sourceSize == 0 )
			return false;

		auto charIter	= CharData.begin();
		auto strIter	= sourceData->begin();

		std::advance( strIter, sourceSize - charSize );

		for( size_t i = 0; i < charSize; i++ )
		{
			if( *charIter != *strIter )
				return false;

			charIter++;
			strIter++;
		}

		return true;
	}

	/*
		String::EndsWith
	*/
	bool String::EndsWith( const String& Source, const String& Pattern )
	{
		auto patternData = Pattern.Data();
		auto sourceData = Source.Data();

		auto sourceSize		= sourceData ? sourceData->size() : 0;
		auto patternSize	= patternData ? patternData->size() : 0;

		if( sourceSize == 0 )					return false;
		else if( patternSize == 0 )				return true;
		else if( sourceSize < patternSize )		return false;

		auto sourceIter		= sourceData->begin();
		auto patternIter	= patternData->begin();
		auto sourcePos		= sourceSize - patternSize;

		std::advance( sourceIter, sourcePos );

		for( size_t i = 0; i < patternSize; i++ )
		{
			if( *sourceIter != *patternIter )
				return false;

			sourceIter++;
			patternIter++;
		}

		return true;
	}

	/*
		String::Compare
	*/
	int String::Compare( const String& lhs, const String& rhs )
	{
		// We need to use character iterators to compare the value of two characters of the same index of both strings
		if( lhs.IsEmpty() && rhs.IsEmpty() ) return 0;

		auto rhs_it = rhs.begin();
		for( auto lhs_it = lhs.begin(); lhs_it != lhs.end(); lhs_it++ )
		{
			// Check if the rhs string has ended
			if( rhs_it == rhs.end() )
			{
				// If the rhs is shorter, then return <0
				return -1;
			}

			// Compare the two characters
			auto lhs_char = *lhs_it;
			auto rhs_char = *rhs_it;

			if( rhs_char < lhs_char )
			{
				return -1;
			}
			else if( rhs_char > lhs_char )
			{
				return 1;
			}
			else
			{
				rhs_it++;
			}
		}

		// If we got here, all characters matched, but lets check if there is more
		// characters left in the rhs string, if so return 1
		if( rhs_it == rhs.end() )
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}

	bool String::IsWhitespace( const Char& inChar )
	{
		return(
			( inChar >= 9 && inChar <= 13 ) ||
			inChar == 32 ||
			inChar == 133 ||
			inChar == 160 );
	}

	bool String::IsWhitespace( const String& inStr )
	{
		// If were empty, then return false
		if( IsEmpty( inStr ) )
			return false;

		for( auto It = inStr.begin(); It != inStr.end(); It++ )
		{
			if( !IsWhitespace( *It ) )
				return false;
		}

		return true;
	}

	bool String::IsWhitespaceOrEmpty( const String& inStr )
	{
		return( IsWhitespace( inStr ) || IsEmpty( inStr ) );
	}

	String String::TrimBegin( const String& inStr )
	{
		String::iterator whitespaceEnd = inStr.begin();
		for( auto It = inStr.begin(); It != inStr.end(); It++ )
		{
			if( !IsWhitespace( *It ) )
			{
				whitespaceEnd = It;
				break;
			}
		}

		if( whitespaceEnd == inStr.begin() )
		{
			return inStr;
		}
		else
		{
			return String( whitespaceEnd, inStr.end() );
		}
	}

	String String::TrimEnd( const String& inStr )
	{
		// We need to find the last non-whitespace character in this string
		String::iterator lastNonWhitespace = inStr.end();
		for( auto It = inStr.begin(); It != inStr.end(); It++ )
		{
			if( !IsWhitespace( *It ) )
			{
				lastNonWhitespace = It;
			}
		}

		// Advance our iterator.. this way the string range includes the final non whitespace character
		if( lastNonWhitespace != inStr.end() )
		{
			std::advance( lastNonWhitespace, 1 );
		}

		// Now, our string goes from the start of the string, until the last non-whitespace character
		if( lastNonWhitespace == inStr.end() )
		{
			return inStr;
		}
		else
		{
			return String( inStr.begin(), lastNonWhitespace );
		}
	}

	String String::TrimBoth( const String& inStr )
	{
		bool firstNonWhiteSpaceFound = false;
		String::iterator firstNonWhiteSpace = inStr.begin();
		String::iterator lastNonWhiteSpace = inStr.end();

		for( auto It = inStr.begin(); It != inStr.end(); It++ )
		{
			// Find the begining of the non-whitespace character
			if( !IsWhitespace( *It ) )
			{
				if( !firstNonWhiteSpaceFound )
				{
					firstNonWhiteSpace = It;
					firstNonWhiteSpaceFound = true;
				}

				lastNonWhiteSpace = It;
			}
		}

		// Advance the last non-whitespace character so we include the final character in the output string
		if( lastNonWhiteSpace != inStr.end() )
		{
			std::advance( lastNonWhiteSpace, 1 );
		}

		return String( firstNonWhiteSpace, lastNonWhiteSpace );
	}


	bool String::IsNumeric( const Char& inChar )
	{
		// We want to check if this character is a number
		return( inChar >= 48 && inChar <= 57 );
	}

	bool String::IsNumeric( const String& inStr )
	{
		if( inStr.IsEmpty() )
			return false;

		for( auto It = inStr.begin(); It != inStr.end(); It++ )
		{
			if( !String::IsNumeric( *It ) )
				return false;
		}

		return true;
	}

	bool String::ToUInt( const String& inStr, uint32& outVal )
	{
		// Trim any trailing or leading whitespace
		String str = String::TrimBoth( inStr );

		if( str.IsEmpty() )
			return false;

		std::vector< uint8 > digits;
		for( auto It = str.begin(); It != str.end(); It++ )
		{
			Char thisChar = *It;
			if( thisChar >= 48 && thisChar <= 57 )
			{
				digits.push_back( (uint8) ( thisChar - 48 ) );
			}
			else
			{
				return false;
			}
		}

		// Now, we iterate throught he digits and build the number
		outVal = 0;
		uint32 digitCount = (uint32)digits.size();

		for( uint32 i = 0; i < digitCount; i++ )
		{
			uint32 thisDigitNumber	= digitCount - i - 1;
			uint8 thisDigitValue	= digits.at( i );

			outVal += ( Math::Pow( 10, thisDigitNumber ) * thisDigitValue );
		}

		return true;
	}

	bool String::ToInt( const String& inStr, int32& outVal )
	{
		// Trim any trailing or leading whitespace
		String str = String::TrimBoth( inStr );

		if( str.IsEmpty() )
			return false;

		// Check if we have a '+' or '-' symbol
		auto firstChar = *( str.begin() );
		bool bNegative = false;

		if( firstChar == 45 )
		{
			bNegative = true;

			auto begin = str.begin();
			std::advance( begin, 1 );

			// If this is the only character.. then we have an invalid number
			if( begin == str.end() )
				return false;

			// Next, build a number from all other characters
			str = String( begin, str.end() );
		}
		else if( firstChar == 43 )
		{
			auto begin = str.begin();
			std::advance( begin, 1 );

			// If this is the only character, then we have an invalid number
			if( begin == str.end() )
				return false;

			str = String( begin, str.end() );
		}

		// Now that we have the symbol trimmed, and figured out if were positive or negative
		// we can use the uint function and apply the sign
		uint32 baseNumber = 0;
		if( !String::ToUInt( str, baseNumber ) )
		{
			return false;
		}

		// Convert unsigned int, to signed int, with possible negative
		if( bNegative )
		{
			if( baseNumber > (uint32)std::numeric_limits< int32 >::max() )
			{
				Console::WriteLine( "[ERROR] String: Attempt to read an int32 from string, but there was overflow!" );
				return false;
			}

			int32 outValue = baseNumber;
			outValue *= -1;

			return outValue;
		}
		else
		{
			return baseNumber;
		}
	}


	/*
		String::ToFloat
	*/
	bool String::ToFloat( const String& inStr, float& outVal )
	{
		// Trim input
		String str = inStr.TrimBoth();

		if( str.IsEmpty() )
		{
			return false;
		}

		// Use stringstream to cast to a float, since we can check if its valid, and not just return 0.f if its not
		std::istringstream sstream( String::GetSTLString( str ) );
		sstream >> outVal;

		return sstream.eof() && !sstream.fail();
	}


	/*
		String::Explode
	*/
	std::vector< String > String::Explode( const String& Source, const Char& Where )
	{
		std::vector< String > Output;

		// Basically, we need to iterate through the string, looking for this character
		String::iterator lastSplit = Source.begin();
		for( auto It = Source.begin(); It != Source.end(); It++ )
		{
			// Check if we found the character
			if( *It == Where )
			{
				String newStr( lastSplit, It );
				Output.push_back( newStr );

				// We want the next iteration to start after the comma
				std::advance( It, 1 );

				// If we hit the end of the string then break out of the loop
				if( It == Source.end() )
					break;

				// Otherwise.. store this position as the start of the next split
				lastSplit = It;
			}
		}

		// Now, check if there was any remaining string data since the last split
		if( lastSplit != Source.end() )
		{
			String newStr( lastSplit, Source.end() );
			Output.push_back( newStr );
		}

		return Output;
	}


	bool String::IsUpper( Char inChar )
	{
		// This really only works for english, maybe we should think about other languages?
		// These functions would get quite large though
		return( inChar >= 65 && inChar <= 90 );
	}


	bool String::IsLower( Char inChar )
	{
		return( inChar >= 97 && inChar <= 122 );
	}


	String String::ToLower( const String& inStr )
	{
		// Optimized method, the code point for ascii characters is unique
		// So, we can just look for the code points and replace them with new ones 
		std::vector< byte > newData;
		auto thisData = inStr.Data();

		if( thisData )
		{
			for( auto It = thisData->begin(); It != thisData->end(); It++ )
			{
				auto pointValue = *It;
				if( pointValue >= 65 && pointValue <= 90 )
				{
					pointValue += 32;
				}

				newData.push_back( pointValue );
			}
		}

		return String( newData, StringEncoding::UTF8 );
	}

	String String::ToUpper( const String& inStr )
	{
		std::vector< byte > newData;
		auto thisData = inStr.Data();

		if( thisData )
		{
			for( auto It = thisData->begin(); It != thisData->end(); It++ )
			{
				auto pointValue = *It;
				if( pointValue >= 97 && pointValue <= 122 )
				{
					pointValue -= 32;
				}

				newData.push_back( pointValue );
			}
		}

		return String( newData, StringEncoding::UTF8 );
	}



	/*
		String::Equals
	*/
	bool String::Equals( const String& First, const String& Second )
	{
		return String::Compare( First, Second ) == 0;
	}

	/*
		String::GetSTLString
	*/
	std::string String::GetSTLString( const String& In, char ReplacementChar /* = (char) 0 */ )
	{
		auto strData = In.Data();
		if( strData )
		{
			return GetSTLString( *strData, ReplacementChar );
		}

		return std::string();
	}

	/*
		String::GetSTLString
	*/
	std::string String::GetSTLString( const std::vector< byte >& In, char ReplacementChar /* = (char) 0 */ )
	{
		std::string Output;
		std::vector< uint32 > CodePoints;
		Encoding::UTF8::BinaryToCodes( In.begin(), In.end(), CodePoints );
		Output.reserve( CodePoints.size() );

		for( auto It = CodePoints.begin(); It != CodePoints.end(); It++ )
		{
			// Check if the code point is within range
			if( *It < 255 )
			{
				Output.push_back( char( *It ) );
			}
			else
			{
				if( ReplacementChar != 0 )
					Output.push_back( ReplacementChar );
			}
		}

		return Output;
	}

	/*
		String::GetSTLWString
	*/
	std::wstring String::GetSTLWString( const String& In )
	{
		std::wstring Output;
		auto strData = In.Data();

		if( strData )
		{
			std::vector< uint32 > CodePoints;
			Encoding::UTF8::BinaryToCodes( strData->begin(), strData->end(), CodePoints );
			Output.reserve( CodePoints.size() );

			for( auto It = CodePoints.begin(); It != CodePoints.end(); It++ )
			{
				Output.push_back( wchar_t( *It ) );
			}
		}

		return Output;
	}

	/*
		String::IsLocalized
	*/
	bool String::IsLocalized( const String& In )
	{
		if( !In.m_Data )
			return false;

		return dynamic_cast< LocalizedStringData* >( In.m_Data.get() );
	}

	bool String::IsBasic( const String& In )
	{
		if( !In.m_Data )
			return true;

		return dynamic_cast< NonCachedStringData* >( In.m_Data.get() );
	}

	/*
		String::GetLocalized
	*/
	String String::GetLocalized( const std::string& inKey, const std::string& inDefault /* = std::string() */, StringEncoding Encoding /* = StringEncoding::ASCII */ )
	{
		return String( std::make_shared< LocalizedStringData >( inKey, inDefault ) );
	}

	/*
		String::GetLocalized
	*/
	String String::GetLocalized( const std::string& inKey, const String& inDefault )
	{
		// Ensure the key is lowercase
		std::string lowerKey;
		lowerKey.resize( inKey.size() );
		std::transform( inKey.begin(), inKey.end(), lowerKey.begin(),
						[] ( unsigned char c ) -> unsigned char { return std::tolower( c ); } );

		// Ensure were not using a localized string as the default
		if( inDefault.IsLocalized() )
		{
			Console::WriteLine( "[ERROR] String Library: Attempt to create a localized string using another localized string as the fallback/default!" );
			return String( std::make_shared< LocalizedStringData >( inKey ) );
		}

		std::shared_ptr< const std::vector< byte > > defaultData = inDefault.m_Data ? inDefault.m_Data->GetData() : nullptr;
		if( defaultData )
		{
			return String( std::make_shared< LocalizedStringData >( inKey, *defaultData ) );
		}
		else
		{
			return String( std::make_shared< LocalizedStringData >( inKey ) );
		}
	}


	/*=========================================================================================================
		String Iterators
	=========================================================================================================*/

	/*
		String::iterator::
	*/
	void String::iterator::ReadThisChar()
	{
		if( !IsCharStart() )
		{
			if( !AdvanceToNextCharStart() )
			{
				// This means we hit the end of the vector, or have no container
				m_ThisChar = Encoding::InvalidChar;
				return;
			}
		}

		// We want to read the first byte in the character so we know the length of the character to expect
		uint8 charLen = 0;
		uint8 thisChar = *m_Iter;
		if( !Encoding::UTF8::CheckForCharStart( thisChar, charLen ) || charLen == 0 )
		{
			// Strange Issue? AdvanceToNextCharStart should have landed us on a valid character... so we will just
			// basically give up here.. dont really know how to handle this yet
			m_ThisChar = Encoding::InvalidChar;
			return;
		}

		// Now, we want to find the start of the next character, gets the bytes in between, and read it into a code point
		std::vector< byte >::const_iterator CharBegin = m_Iter;
		std::vector< byte >::const_iterator CharEnd = m_Iter;

		for( auto It = m_Iter + 1;; )
		{
			if( It == m_Target->end() || !Encoding::UTF8::CheckIfContinuationByte( *It ) )
			{
				CharEnd = It;
				break;
			}

			It++;
		}

		// Now, were going to check the distance between the two iterators
		auto iterDistance = std::distance( CharBegin, CharEnd );

		// If were missing data, then we put out an invalid character
		if( iterDistance < charLen )
		{
			m_ThisChar = Encoding::InvalidChar;
			return;
		}
		else if( iterDistance > charLen )
		{
			// Were only going to use the bytes that the encoding said we should
			CharEnd = CharBegin;
			std::advance( CharEnd, charLen );
		}

		// Now, we have iterators to the start and end of the character info, so we can use the UTF8 library to read it in
		if( !Encoding::UTF8::ReadChar( CharBegin, CharEnd, m_ThisChar ) )
		{
			m_ThisChar = Encoding::InvalidChar;
		}
	}

	/*
		String::iterator::AdvanceToNextCharStart
	*/
	bool String::iterator::AdvanceToNextCharStart()
	{
		// We want to return false if we either hit the end of the container, or have no container
		if( !m_Target || m_Iter == m_Target->end() )
			return false;

		for( auto It = m_Iter + 1;; )
		{
			// Check if we hit the end of the string
			if( It == m_Target->end() )
			{
				break;
			}

			// Check if we found the start of another character
			if( Encoding::UTF8::CheckForCharStartFast( *It ) )
			{
				m_Iter = It;
				return true;
			}
			else
			{
				It++;
			}
		}

		// We should never hit this point
		m_Iter = m_Target->end();
		return false;
	}

	/*
		String::iterator::IsCharStart
	*/
	bool String::iterator::IsCharStart()
	{
		if( !m_Target || m_Iter == m_Target->end() )
			return false;

		return Encoding::UTF8::CheckForCharStartFast( *m_Iter );
	}

	/*
		String::iterator::IsAtEnd
	*/
	bool String::iterator::IsAtEnd()
	{
		if( !m_Target || m_Iter == m_Target->end() )
			return true;

		return false;
	}

	/*
		String::iterator::iterator
	*/
	String::iterator::iterator( std::shared_ptr< const std::vector< byte > >& Target )
		: m_Target( Target ), m_ThisChar( Encoding::InvalidChar )
	{}

	/*
		String::iterator::iterator
	*/
	String::iterator::iterator( std::shared_ptr< const std::vector< byte > >& Target, std::vector< byte >::const_iterator Iter ) 
		: m_Target( Target ), m_Iter( Iter ), m_ThisChar( Encoding::InvalidChar )
	{
		ReadThisChar();
	}

	/*
		String::iterator::iterator
	*/
	String::iterator::iterator( const iterator& Other )
		: m_Target( Other.m_Target ), m_Iter( Other.m_Iter ), m_ThisChar( Other.m_ThisChar )
	{}

	/*
		String::iterator::~iterator
	*/
	String::iterator::~iterator()
	{}

	/*
		String::iterator::operator++
	*/
	String::iterator& String::iterator::operator++()
	{ 
		AdvanceToNextCharStart(); 
		ReadThisChar(); 
		return *this; 
	}

	/*
		String::iterator::operator++
	*/
	String::iterator String::iterator::operator++(int) 
	{ 
		iterator ret = *this; 
		++( *this ); 
		return ret; 
	}

	/*
		String::iterator::operator==
	*/
	bool String::iterator::operator==( iterator Other ) const 
	{
		// Were going to assert that both iterators are from the same container
#if STRING_DEBUG_LEVEL == 0
		HYPERION_VERIFY( m_Target == Other.m_Target, "String iterators originate from different string objects!" );
#endif
		if( !m_Target )
		{
			return Other.m_Target ? false : true;
		}
		else
		{
			return m_Target == Other.m_Target && m_Iter == Other.m_Iter;
		}
	}

	/*
		String::iterator::operator!=
	*/
	bool String::iterator::operator!=( iterator Other ) const 
	{
#if STRING_DEBUG_LEVEL == 0
		HYPERION_VERIFY( m_Target == Other.m_Target, "String iterators originate from different string objects!" );
#endif
		if( !m_Target )
		{
			return Other.m_Target ? true : false;
		}
		else
		{
			return m_Target != Other.m_Target || m_Iter != Other.m_Iter;
		}
	}

	/*
		String::iterator::operator=
	*/
	void String::iterator::operator=( const iterator& Other )
	{
		m_Target = Other.m_Target;
		m_Iter = Other.m_Iter;
		m_ThisChar = Other.m_ThisChar;
	}

	/*
		String::iterator::operator-
	*/
	size_t String::iterator::operator-( const iterator& Other )
	{
		// Basically, were going to assume the other iterator is first.. otherwise we will hit the end of the
		// string, and in that case we return 0, nothing should throw an exception
		auto otherCopy = Other;
		size_t Output = 0;

		if( operator==( otherCopy ) )
			return 0;

		// Basically, we want to break for two situations..
		// 1) otherCopy hits the end of the string
		// 2) otherCopy == *this
		for( ; operator!=( otherCopy ) && !otherCopy.IsAtEnd(); otherCopy++ )
		{
			Output++;
		}

		return Output;
	}

	/*
		String::iterator::Distance
	*/
	size_t String::iterator::Distance( const iterator& Other )
	{
		return operator-( Other );
	}

	/*
		String::iterator::operator*
	*/
	Char String::iterator::operator*()
	{
		return m_ThisChar;
	}

	/*
		Specialized ToString Definitions
	*/
	String ToString( const char* in )
	{
		return String( in );
	}

	String ToString( const std::string& in )
	{
		return String( in );
	}

	String ToString( const String& in )
	{
		return in;
	}

}