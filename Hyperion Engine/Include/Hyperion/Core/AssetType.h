#pragma once

#include "Hyperion/Hyperion.h"

#include <functional>
#include <memory>


namespace Hyperion
{
	// Forward Declaration 
	// Avoiding circular dependance
	class AssetBase;

	class AssetType
	{

	public:

		// Constructor definition in AssetManager.cpp
		AssetType() = delete;
		AssetType( uint32 inIdentifier, const String& inName, const String& inExt,
				   std::function< std::shared_ptr< AssetBase >( std::unique_ptr< File >&, const String&, uint32, uint64, uint64 ) > inLoadFunc );
	};

}