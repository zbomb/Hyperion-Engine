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

/*
*	TODO:
*	* We need to rethink how we store materials on the render thread
*	* The old system of using a map with Hyperion::String took around 0.2ms to lookup a texture from the material
*	* This is WAYYY too slow, when it comes to many thousands of primitives per frame
*	* We need quick access to the values we need from a material.. almost like a 'render material' 
*/


namespace Hyperion
{

	/*
	*	Material Class
	*/
	class RMaterial
	{

	protected:

		uint32 m_Identifier;

		std::map< std::string, std::any > m_Values;
		std::map< std::string, std::shared_ptr< RTexture2D > > m_Textures;

		std::shared_ptr< RTexture2D > m_BaseMap;
		bool m_bTransparent;

	public:

		// Some types we want shorthand for..
		using valIter = std::map< std::string, std::any >::const_iterator;
		using texIter = std::map< std::string, std::shared_ptr< RTexture2D > >::const_iterator;

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
		inline std::shared_ptr< RTexture2D > GetBaseMap() const { return m_BaseMap; }

		void CacheTextures()
		{
			m_BaseMap		= GetTexture( "base_map" );
			m_bTransparent	= GetBool( "enable_alpha", false );
		}

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
		void AddTexture( const std::string& inKey, const std::shared_ptr< RTexture2D >& inTexture )
		{
			m_Textures.emplace( inKey, inTexture );
		}

		void RemoveTexture( const std::string& inKey )
		{
			m_Textures.erase( inKey );
		}

		bool TextureExists( const std::string& inKey ) const
		{
			auto it = m_Textures.find( inKey );
			return it != m_Textures.end() && it->second;
		}

		std::shared_ptr< RTexture2D > GetTexture( const std::string& inKey ) const
		{
			auto it = m_Textures.find( inKey );
			return it != m_Textures.end() ? it->second : nullptr;
		}

		bool AreTexturesLoaded() const
		{
			return m_BaseMap ? m_BaseMap->IsValid() : false;
		}

		bool IsTranslucent() const
		{
			return m_bTransparent;
		}

		/*
		*	Value Functions
		*/
		void SetValue( const std::string& inKey, const std::any& inValue )
		{
			auto entry = m_Values.find( inKey );
			if( entry == m_Values.end() ) { return; }

			entry->second = inValue;
		}

		std::any GetValue( const std::string& inKey ) const
		{
			auto entry = m_Values.find( inKey );
			if( entry == m_Values.end() ) { return std::any(); }

			return entry->second;
		}

		bool GetBool( const std::string& inKey, bool bDefault = false ) const
		{
			auto any = GetValue( inKey );
			bool* val = std::any_cast<bool>( &any );
			return val ? *val : bDefault;
		}

		int32 GetInt( const std::string& inKey, int32 iDefault = 0 ) const
		{
			auto any = GetValue( inKey );
			int32* val = std::any_cast<int32>( &any );
			return val ? *val : iDefault;
		}

		uint32 GetUInt( const std::string& inKey, uint32 iDefault = 0 ) const
		{
			auto any = GetValue( inKey );
			uint32* val = std::any_cast<uint32>( &any );
			return val ? *val : iDefault;
		}

		std::string GetString( const std::string& inKey, const std::string& sDefault = std::string ( ) )
		{
			auto any = GetValue( inKey );
			std::string* val = std::any_cast<std::string>( &any );
			return val ? *val : sDefault;
		}

		float GetFloat( const std::string& inKey, float fDefault = 0.f ) const
		{
			auto any = GetValue( inKey );
			float* val = std::any_cast<float>( &any );
			return val ? *val : fDefault;
		}

		bool ValueExists( const std::string& inKey ) const
		{
			return GetValue( inKey ).has_value();
		}

		void ClearValue( const std::string& inKey )
		{
			m_Values.erase( inKey );
		}

	};

}