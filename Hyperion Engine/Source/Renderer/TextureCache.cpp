/*==================================================================================================
	Hyperion Engine
	Source/Renderer/TextureCache.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/TextureCache.h"
#include "Hyperion/Renderer/IGraphics.h"
#include "Hyperion/Renderer/Types/ITexture.h"


namespace Hyperion
{

	TextureCache::TextureCache( const std::shared_ptr< IGraphics >& inAPI )
		: m_API( inAPI )
	{
		HYPERION_VERIFY( inAPI, "Attempt to create texture cache with null api!" );
	}


	TextureCache::~TextureCache()
	{
		m_API.reset();
		Clear();
	}


	bool TextureCache::IsCached( const String& inIdentifier )
	{
		std::shared_lock< std::shared_mutex > lock( m_Mutex );

		auto entry = m_Cache.find( inIdentifier );
		return entry != m_Cache.end() && entry->second;
	}


	std::shared_ptr< ITexture2D > TextureCache::Get( const String& inIdentifier )
	{
		std::shared_lock< std::shared_mutex > lock( m_Mutex );
		auto entry = m_Cache.find( inIdentifier );
		if( entry == m_Cache.end() || !entry->second )
			return nullptr;

		return entry->second;
	}

	bool TextureCache::Cache( const String& inIdentifier, const RawImageData& inData )
	{
		std::shared_ptr< IGraphics > api = m_API.expired() ? nullptr : m_API.lock();
		auto identifier = inIdentifier.ToLower().TrimBoth();

		if( !api )
		{
			Console::WriteLine( "[ERROR] TextureCache: Failed to cache texture '", identifier, "'.. api was invalid" );
			return false;
		}

		// Validate parameters
		if( inData.Width == 0 || inData.Height == 0 )
		{
			Console::WriteLine( "[ERROR] TextureCache: Failed to create texture '", identifier, "'.. invalid texture size! (", inData.Width, "x", inData.Height, ")" );
			return false;
		}

		if( inData.Data.size() != ( (uint64)inData.Width * (uint64)inData.Height * 4 ) )
		{
			Console::WriteLine( "[ERROR] TextureCache: Failed to create texture '", identifier, "'.. size of the data didnt match the resolution! (", inData.Data.size(), "bytes for a ", inData.Width, "x", inData.Height, ") image)" );
			return false;
		}

		// Create our texture parameters
		Texture2DParameters Params;

		Params.CanCPURead	= false;
		Params.Dynamic		= false;
		Params.Format		= TextureFormat::RGBA_8BIT_UNORM;
		Params.MipLevels	= 1; // TODO: Allow mips!
		Params.Target		= TextureBindTarget::Shader;
		Params.Width		= inData.Width;
		Params.Height		= inData.Height;
		Params.RowDataSize	= 4 * inData.Width; // Were using 4 byte pixels (one for each channel, RGBA) and mult by the width to get row size in bytes
		Params.Data			= inData.Data.data();

		// Make our API call to create this texture
		auto newTexture = api->CreateTexture2D( Params );
		if( !newTexture )
		{
			Console::WriteLine( "[ERROR] TextureCache: Failed to create texture '", identifier, "' the api returned null!" );
			return false;
		}

		// Now we need to aquire a lock and insert this into our cache
		{
			std::unique_lock< std::shared_mutex > lock( m_Mutex );

			// If this texture already exists, then shutdown the old one
			if( m_Cache[ identifier ] )
			{
				m_Cache[ identifier ]->Shutdown();
			}

			m_Cache[ identifier ] = newTexture;
		}

		return true;
	}


	bool TextureCache::UnCache( const String& inIdentifier )
	{
		std::unique_lock< std::shared_mutex > lock( m_Mutex );

		auto entry = m_Cache.find( inIdentifier );
		if( entry == m_Cache.end() )
			return false;

		if( entry->second )
		{
			entry->second->Shutdown();
		}

		m_Cache.erase( entry );
		return true;
	}

	void TextureCache::Clear()
	{
		std::unique_lock< std::shared_mutex > lock( m_Mutex );

		for( auto It = m_Cache.begin(); It != m_Cache.end(); It++ )
		{
			if( It->second )
			{
				It->second->Shutdown();
			}
		}

		m_Cache.clear();
	}

}
