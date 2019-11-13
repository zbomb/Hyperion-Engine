/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/VFS.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/String.h"


namespace Hyperion
{

	class VFS
	{

	public:

		VFS() = delete;

		static bool IsMounted( const String& chunkName );
		static bool MountChunk( const String& chunkName );
		static bool UnmountChunk( const String& chunkName );
		static std::vector< String > GetMountedChunks();

		static bool AssetExists( const String& filePath );
		static bool AssetCached( const String& filePath );
		
		


	};

}