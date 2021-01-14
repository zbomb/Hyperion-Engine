/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resource/Material.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resource/Texture.h"
#include "Hyperion/Assets/MaterialAsset.h"

#include <any>


namespace Hyperion
{

	class RMaterial
	{

	protected:

		std::map< String, std::any > m_Values;
		std::map< String, std::shared_ptr< RTexture2D > > m_Textures;

		uint32 m_Identifier;

	public:

		using valIter = std::map< String, std::any >::const_iterator;

		RMaterial() = delete;
		RMaterial( valIter valBegin, valIter valEnd, uint32 inId )
			: m_Values( valBegin, valEnd ), m_Identifier( inId )
		{}

		inline uint32 GetIdentifier() const { return m_Identifier; }

		void AddTexture( const String& inKey, const std::shared_ptr< RTexture2D >& inTex )
		{
			if( inKey.IsWhitespaceOrEmpty() ) { return; }
			m_Textures.emplace( inKey.ToLower(), inTex );
		}

		void RemoveTexture( const String& inKey )
		{
			if( inKey.IsWhitespaceOrEmpty() ) { return; }
			m_Textures.erase( inKey.ToLower() );
		}

		std::shared_ptr< RTexture2D > GetTexture( const String& inKey )
		{
			if( inKey.IsWhitespaceOrEmpty() ) { return nullptr; }

			auto entry = m_Textures.find( inKey.ToLower() );
			if( entry == m_Textures.end() ) { return nullptr; }

			return entry->second;
		}

		bool TextureExists( const String& inKey )
		{
			return GetTexture( inKey ) != nullptr;
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

		void SetValue( const String& inKey, const std::any& inValue )
		{
			if( inKey.IsWhitespaceOrEmpty() ) { return; }
			
			auto entry = m_Values.find( inKey.ToLower() );
			if( entry == m_Values.end() ) { return; }

			entry->second = inValue;
		}

		std::any GetValue( const String& inKey )
		{
			if( inKey.IsWhitespaceOrEmpty() ) { return std::any(); }

			auto entry = m_Values.find( inKey.ToLower() );
			if( entry == m_Values.end() ) { return std::any(); }

			return entry->second;
		}

		bool GetBool( const String& inKey, bool bDefault = false )
		{
			auto any = GetValue( inKey );
			bool* val = std::any_cast< bool >( &any );
			return val ? *val : bDefault;
		}

		int32 GetInt( const String& inKey, int32 iDefault = 0 )
		{
			auto any = GetValue( inKey );
			int32* val = std::any_cast<int32>( &any );
			return val ? *val : iDefault;
		}

		uint32 GetUInt( const String& inKey, uint32 iDefault = 0 )
		{
			auto any = GetValue( inKey );
			uint32* val = std::any_cast<uint32>( &any );
			return val ? *val : iDefault;
		}

		String GetString( const String& inKey, const String& sDefault = String() )
		{
			auto any = GetValue( inKey );
			String* val = std::any_cast< String >( &any );
			return val ? *val : sDefault;
		}

		float GetFloat( const String& inKey, float fDefault = 0.f )
		{
			auto any = GetValue( inKey );
			float* val = std::any_cast< float >( &any );
			return val ? *val : fDefault;
		}

		bool ValueExists( const String& inKey )
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