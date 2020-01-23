/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/TextureCache.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include <shared_mutex>


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class IGraphics;
	class ITexture2D;
	struct RawImageData;


	class TextureCache
	{

	private:

		std::weak_ptr< IGraphics > m_API;
		std::map< String, std::shared_ptr< ITexture2D > > m_Cache;
		std::shared_mutex m_Mutex;

	public:

		TextureCache() = delete;
		TextureCache( const std::shared_ptr< IGraphics >& inAPI );

		~TextureCache();

		bool IsCached( const String& inIdentifier );

		std::shared_ptr< ITexture2D > Get( const String& inIdentifier );

		bool Cache( const String& inIdentifier, const RawImageData& inData );

		bool UnCache( const String& inIdentifier );

		void Clear();

	};

}