/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/String.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"

#include <map>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <string>

// String Debug Levels
// 0: No Debugging
// 1: Debugging
#define STRING_DEBUG_LEVEL 0

namespace Hyperion
{
	/*----------------------------------------------------------------------------------------
		Enums/Structures
	----------------------------------------------------------------------------------------*/
	enum class StringEncoding
	{
		ASCII	= 0,
		UTF8	= 1,
		UTF16	= 2,
		UTF32	= 3,
		AUTO	= 4
	};

	struct LanguageInfo
	{
		std::vector< byte > DisplayName;
		std::string Key;
		LanguageID Identifier;
	};

	/*----------------------------------------------------------------------------------------
		Localization
	----------------------------------------------------------------------------------------*/
	class Localization
	{

	public:

		/*------------------------------------------------
			Localization Cache
		------------------------------------------------*/
		class Cache
		{

		public:

			/*
				Internal Instance Structure
			*/
			struct Instance
			{
				std::shared_ptr< const std::vector< byte > > m_Data;
				std::atomic< uint32 > m_RefCount;
				std::string m_LocalizationKey;
				LanguageID m_Lang;
			};

			/*
				Instance List
			*/
			static std::unordered_map< std::string, std::shared_ptr< Instance > > m_Values;

			/*
				Localization::Cache::CreateInstance
			*/
			static std::shared_ptr< Instance > CreateInstance( const std::string& inStr );

			/*
				Localization::Cache::DestroyInstance
			*/
			static void DestroyInstance( std::shared_ptr< Instance >& In );

		};

		/*------------------------------------------------
			Localization Language System 
		------------------------------------------------*/
	private:

		/*
			Private Static Members
		*/
		static std::map< uint16, LanguageInfo > m_Languages;
		static LanguageID m_LastLangID;

	public:

		/*
			Default Language 
		*/
		static LanguageInfo Language_None;

		/*
			Localization::GetLanguage
		*/
		static const LanguageInfo& GetLanguage( LanguageID Identifier );
		static const LanguageInfo& GetLanguage( const std::string& Key );

		/*
			Localization::LanguageExists
		*/
		static bool LanguageExists( LanguageID Identifier );
		static bool LanguageExists( const std::string& Key );

		/*
			Localization::AddLanguage
		*/
		static LanguageID AddLanguage( LanguageInfo& In );

		/*
			Localization::CreateOrUpdateDefinition
		*/
		static bool CreateOrUpdateDefinition( LanguageID Lang, const std::string& Key, std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End );

		/*
			Localization::LookupDefinition
		*/
		static std::shared_ptr< Cache::Instance > LookupDefinition( const std::string& Key );

		/*
			Localization::ClearLanguage
		*/
		static void ClearLanguage( LanguageID Lang );

		/*
			Localization::ClearDefinition
		*/
		static bool ClearDefinition( const std::string& Key );

		/*
			Localization::DebugCache
		*/
		static void DebugCache();


		friend class String;
	};


	/*----------------------------------------------------------------------------------------
		String Class
	----------------------------------------------------------------------------------------*/
	class String
	{
		
	public:

		/*--------------------------------------------------------------------------------
			String Cache
		--------------------------------------------------------------------------------*/
		class Cache
		{
		
		public:

			/*
				Internal Instance Structure
			*/
			struct Instance
			{
				std::shared_ptr< const std::vector< byte > > m_Data;
				std::atomic< uint32 > m_RefCount;
			};

			/*
				Instance Cache
			*/
			static std::map< const std::vector< byte >, std::shared_ptr< Instance > > m_Values;

			/*
				String::Cache::CreateInstance
			*/
			static std::shared_ptr< Instance > CreateInstance( const std::vector< byte >& In );

			/*
				String::Cache::DestroyInstance
			*/
			static void DestroyInstance( std::shared_ptr< Instance >& In );

			/*
				String::Cache::PrintDebugInfo
			*/
			static void PrintDebugInfo();
			
		};


		/*--------------------------------------------------------------------------------
			IStringData
		--------------------------------------------------------------------------------*/
		struct IStringData
		{
			virtual std::shared_ptr< const std::vector< byte > > GetData() = 0;
		};

		/*--------------------------------------------------------------------------------
			NonLocalizedStringData
		--------------------------------------------------------------------------------*/
		struct NonLocalizedStringData : public IStringData
		{
			/*
				Data Members
			*/
			std::shared_ptr< Cache::Instance > m_Ref;
			std::shared_ptr< const std::vector< byte > > GetData();

			/*
				Constructors
			*/
			NonLocalizedStringData();
			NonLocalizedStringData( const NonLocalizedStringData& Other );
			NonLocalizedStringData( const std::vector< byte >& inData, StringEncoding Encoding );
			NonLocalizedStringData( const std::vector< byte >& inData );

			/*
				Destructor
			*/
			~NonLocalizedStringData();

			/*
				Assignment operator
			*/
			void operator=( const NonLocalizedStringData& Other );
		};

		/*--------------------------------------------------------------------------------
			LocalizedStringData
		--------------------------------------------------------------------------------*/
		struct LocalizedStringData : public IStringData
		{
			/*
				Data Members
			*/
			std::shared_ptr< Localization::Cache::Instance > m_LocalizedRef;
			std::shared_ptr< Cache::Instance > m_NonLocalizedRef;
			std::shared_ptr< const std::vector< byte > > GetData();

			/*
				Constructors
			*/
			LocalizedStringData();
			LocalizedStringData( const LocalizedStringData& Other );
			LocalizedStringData( const std::string& inKey );
			LocalizedStringData( const std::string& inKey, const std::vector< byte >& inDefault, StringEncoding Enc = StringEncoding::ASCII );
			LocalizedStringData( const std::string& inKey, const std::string& inDefault, StringEncoding Enc = StringEncoding::ASCII );
			LocalizedStringData( const std::string& inKey, const char* inDefault, StringEncoding Enc = StringEncoding::ASCII );
			LocalizedStringData( const char* inKey );
			LocalizedStringData( const char* inKey, const std::vector< byte >& inDefault, StringEncoding Enc = StringEncoding::ASCII );
			LocalizedStringData( const char* inKey, const std::string& inDefault, StringEncoding Enc = StringEncoding::ASCII );
			LocalizedStringData( const char* inKey, const char* inDefault, StringEncoding Enc = StringEncoding::ASCII );

			/*
				Destructor
			*/
			~LocalizedStringData();

			/*
				Assignment operator
			*/
			void operator=( const LocalizedStringData& Other );
		};

		/*--------------------------------------------------------------------------------
			Iterators
		--------------------------------------------------------------------------------*/
	public:

		class iterator
		{
		private:

			/*
				Data Members
			*/
			std::shared_ptr< const std::vector< byte > > m_Target;
			std::vector< byte >::const_iterator m_Iter;
			Char m_ThisChar;

			/*
				String::iterator::ReadThisChar
			*/
			void ReadThisChar();

			/*
				String::iterator::AdvanceToNextCharStart
			*/
			bool AdvanceToNextCharStart();

			/*
				String::iterator::IsCharStart
			*/
			bool IsCharStart();

			/*
				String::iterator::IsAtEnd
			*/
			bool IsAtEnd();

		public:

			/*
				Constructors
			*/
			iterator( std::shared_ptr< const std::vector< byte > >& Target );
			iterator( std::shared_ptr< const std::vector< byte > >& Target, std::vector< byte >::const_iterator Iter );
			iterator( const iterator& Other );

			/*
				Destructor
			*/
			~iterator();

			/*
				++ Operators
			*/
			iterator& operator++();
			iterator operator++( int );

			/*
				Comparison Operators
			*/
			bool operator==( iterator Other ) const;
			bool operator!=( iterator Other ) const;

			/*
				Assignment Operator
			*/
			void operator=( const iterator& Other );

			/*
				Distance Functions
			*/
			size_t operator-( const iterator& Other );
			size_t Distance( const iterator& Other );

			/*
				Dereference Operator
			*/
			Char operator*();

			/*
				Iterator Traits
			*/
			using difference_type = uint32;
			using value_type = Char;
			using pointer = const Char*;
			using reference = const Char&;
			using iterator_category = std::forward_iterator_tag;

			friend class String;
		};


		/*--------------------------------------------------------------------------------
			String Members
		--------------------------------------------------------------------------------*/

	private:

		std::shared_ptr< IStringData > m_Data;

		/*--------------------------------------------------------------------------------
			Constructors
		--------------------------------------------------------------------------------*/

	public:

		String();
		String( nullptr_t );
		String( const std::shared_ptr< IStringData >& inRawData );
		String( const std::vector< byte >& inData, StringEncoding Enc = StringEncoding::ASCII );
		String( const std::string& inStr, StringEncoding Enc = StringEncoding::ASCII );
		String( const char* inStr, StringEncoding Enc = StringEncoding::ASCII );
		String( const iterator& Begin, const iterator& End );
		String( const String& Other ); 

		/*--------------------------------------------------------------------------------
			Member Functions
		--------------------------------------------------------------------------------*/

	public:

		/*
			String::Data
		*/
		std::shared_ptr< const std::vector< byte > > Data() const;

		/*
			String::operator=
		*/
		void operator=( const String& Other );

		/*
			String::IsEmpty
		*/
		bool IsEmpty() const;

		/*
			String::begin
		*/
		iterator begin() const;

		/*
			String::end
		*/
		iterator end() const;


		/*--------------------------------------------------------------------------------
			String Library
		--------------------------------------------------------------------------------*/

	protected:

		/*
			String::ChangeEncoding_Impl
		*/
		static const std::vector< byte > ChangeEncoding_Impl( const std::vector< byte >& Source, StringEncoding SourceEncoding, StringEncoding TargetEncoding );

		/*
			String::Find_Impl
		*/
		static iterator Find_Impl( const String& InStr, const std::vector< byte >& inSequence );

	public:

		/*
			String::GetSTLString
		*/
		static std::string GetSTLString( const String& In, char ReplacementChar = (char) 0 );
		static std::string GetSTLString( const std::vector< byte >& In, char ReplacementChar = (char) 0 );

		/*
			String::GetSTLWString
		*/
		static std::wstring GetSTLWString( const String& In );

		/*
			String::IsLocalized
		*/
		static bool IsLocalized( const String& In );

		/*
			String::GetLocalized
		*/
		static String GetLocalized( const std::string& inKey, const std::string& inDefault = std::string(), StringEncoding Encoding = StringEncoding::ASCII );
		static String GetLocalized( const std::string& inKey, const String& inDefault );

		/*
			String::Find
		*/
		static iterator Find( const String& InStr, const Char& InChar );
		static iterator Find( const String& Source, const String& Sequence );

		/*
			String::Length
		*/
		static size_t Length( const String& InStr );

		/*
			String::ByteCount
		*/
		static size_t ByteCount( const String& InStr );

		/*
			String::Append
		*/
		static String Append( const String& First, const String& Other );

		/*
			String::Prepend
		*/
		static String Prepend( const String& First, const String& Prefix );

		/*
			String::SubStr
		*/
		static String SubStr( const String& Source, size_t Start, size_t Length );

		/*
			String::IsEmpty
		*/
		static bool IsEmpty( const String& Source );

		/*
			String::StartsWith
		*/
		static bool StartsWith( const String& Source, const Char& ToCheck );
		static bool StartsWith( const String& Source, const String& Pattern );

		/*
			String::EndsWith
		*/
		static bool EndsWith( const String& Source, const Char& ToCheck );
		static bool EndsWith( const String& Source, const String& Pattern );

		/*
			String::Equals
		*/
		static bool Equals( const String& First, const String& Second );

		/*
			Member Functions For String Library Functions
		*/
		inline size_t Length() const { return String::Length( *this ); }
		inline size_t ByteCount() const { return String::ByteCount( *this ); }
		inline iterator Find( const Char& In ) const { return String::Find( *this, In ); }
		inline iterator Find( const String& Sequence ) const { return String::Find( *this, Sequence ); }
		inline String Append( const String& Other ) const { return String::Append( *this, Other ); }
		inline String Prepend( const String& Prefix ) const { return String::Prepend( *this, Prefix ); }
		inline String SubStr( size_t Start, size_t Length ) const { return String::SubStr( *this, Start, Length ); }
		inline bool StartsWith( const Char& toChech ) const { return String::StartsWith( *this, toChech ); }
		inline bool StartsWith( const String& Pattern ) const { return String::StartsWith( *this, Pattern ); }
		inline bool EndsWith( const char& ToCheck ) const { return String::EndsWith( *this, ToCheck ); }
		inline bool EndsWith( const String& Pattern ) const { return String::EndsWith( *this, Pattern ); }
		inline bool Equals( const String& Other ) const { return String::Equals( *this, Other ); }
		inline bool IsLocalized() const { return String::IsLocalized( *this ); }

		/*
			Operator Overloads
		*/
		inline String operator+( const String& Other ) const { return String::Append( *this, Other ); }
		inline bool operator==( const String& Other ) const { return String::Equals( *this, Other ); }
		inline bool operator!=( const String& Other ) const { return !String::Equals( *this, Other ); }

	};
}

/*
	Operator '<<' Overload
*/
std::ostream& operator<<( std::ostream& inStream, const Hyperion::String& inStr );