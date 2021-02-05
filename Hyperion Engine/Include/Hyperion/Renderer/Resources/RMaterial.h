/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resources/RMaterial.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Assets/MaterialAsset.h"
#include "Hyperion/Renderer/Resources/RTexture.h"

#include <any>


namespace Hyperion
{

	/*
	*	Material Class
	*/
	class RMaterial
	{

	protected:

		uint32 m_Identifier;

		std::map< String, std::any > m_Values;
		std::map< String, std::shared_ptr< RTexture2D > > m_Textures;

	public:

		// Some types we want shorthand for..
		using valIter = std::map< String, std::any >::const_iterator;
		using texIter = std::map< String, std::shared_ptr< RTexture2D > >::const_iterator;

		/*
		*	Constructors/Destructors
		*/
		RMaterial() = delete;

		RMaterial( uint32 inIdentifier )
			: m_Identifier( inIdentifier )
		{}

		RMaterial( uint32 inIdentifier, valIter valuesBegin, valIter valuesEnd )
			: m_Identifier( inIdentifier ), m_Values( valuesBegin, valuesEnd )
		{}

		RMaterial( uint32 inIdentifier, valIter valuesBegin, valIter valuesEnd, texIter texBegin, texIter texEnd )
			: m_Identifier( inIdentifier ), m_Values( valuesBegin, valuesEnd ), m_Textures( texBegin, texEnd )
		{}

		~RMaterial()
		{
			Shutdown();
		}

		/*
		*	Getters
		*/
		inline uint32 GetIdentifier() const { return m_Identifier; }

		/*
		*	Shutdown Function
		*/
		void Shutdown()
		{
			m_Values.clear();
			m_Textures.clear();
		}

		/*
		*	Texture Functions
		*/
		void AddTexture( const String& inKey, const std::shared_ptr< RTexture2D >& inTexture )
		{
			if( inKey.IsWhitespaceOrEmpty() ) { return; }
			m_Textures.emplace( inKey.ToLower(), inTexture );
		}

		void RemoveTexture( const String& inKey )
		{
			if( inKey.IsWhitespaceOrEmpty() ) { return; }
			m_Textures.erase( inKey.ToLower() );
		}

		bool TextureExists( const String& inKey ) const
		{
			if( inKey.IsWhitespaceOrEmpty() ) { return false; }
			auto it = m_Textures.find( inKey.ToLower() );
			return it != m_Textures.end() && it->second;
		}

		std::shared_ptr< RTexture2D > GetTexture( const String& inKey ) const
		{
			if( inKey.IsWhitespaceOrEmpty() ) { return nullptr; }
			auto it = m_Textures.find( inKey.ToLower() );
			return it != m_Textures.end() ? it->second : nullptr;
		}

		bool AreTexturesLoaded() const
		{
			for( auto it = m_Textures.begin(); it != m_Textures.end(); it++ )
			{
				if( !it->second || !it->second->IsValid() )
				{
					return false;
				}
			}

			return true;
		}

		bool IsTranslucent() const
		{
			return GetBool( "enable_alpha", false );
		}

		/*
		*	Value Functions
		*/
		void SetValue( const String& inKey, const std::any& inValue )
		{
			if( inKey.IsWhitespaceOrEmpty() ) { return; }

			auto entry = m_Values.find( inKey.ToLower() );
			if( entry == m_Values.end() ) { return; }

			entry->second = inValue;
		}

		std::any GetValue( const String& inKey ) const
		{
			if( inKey.IsWhitespaceOrEmpty() ) { return std::any(); }

			auto entry = m_Values.find( inKey.ToLower() );
			if( entry == m_Values.end() ) { return std::any(); }

			return entry->second;
		}

		bool GetBool( const String& inKey, bool bDefault = false ) const
		{
			auto any = GetValue( inKey );
			bool* val = std::any_cast<bool>( &any );
			return val ? *val : bDefault;
		}

		int32 GetInt( const String& inKey, int32 iDefault = 0 ) const
		{
			auto any = GetValue( inKey );
			int32* val = std::any_cast<int32>( &any );
			return val ? *val : iDefault;
		}

		uint32 GetUInt( const String& inKey, uint32 iDefault = 0 ) const
		{
			auto any = GetValue( inKey );
			uint32* val = std::any_cast<uint32>( &any );
			return val ? *val : iDefault;
		}

		String GetString( const String& inKey, const String& sDefault = String() )
		{
			auto any = GetValue( inKey );
			String* val = std::any_cast<String>( &any );
			return val ? *val : sDefault;
		}

		float GetFloat( const String& inKey, float fDefault = 0.f ) const
		{
			auto any = GetValue( inKey );
			float* val = std::any_cast<float>( &any );
			return val ? *val : fDefault;
		}

		bool ValueExists( const String& inKey ) const
		{
			return GetValue( inKey ).has_value();
		}

		void ClearValue( const String& inKey )
		{
			if( inKey.IsWhitespaceOrEmpty() ) { return; }
			m_Values.erase( inKey.ToLower() );
		}

	};

}