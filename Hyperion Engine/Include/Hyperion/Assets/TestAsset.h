/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Assets/TestAsset.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Asset.h"
#include "Hyperion/Core/AssetLoader.h"


namespace Hyperion
{
	
	class TestAsset : public Asset
	{

	public:

		TestAsset();
		TestAsset( const String& );
		~TestAsset();

		virtual String GetAssetName() const;

		String m_Data;

	};

	template<>
	inline std::shared_ptr< Asset > AssetLoader::Load< TestAsset >( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
	{
		std::vector< byte > rawData( Begin, End );
		return std::shared_ptr< Asset >( new TestAsset( String( rawData, StringEncoding::ASCII ) ) );
	}

	template<>
	inline std::shared_ptr< Asset > AssetLoader::Stream< TestAsset >( AssetStream& inStream )
	{
		// Were going to take this data in as a string
		std::vector< byte > rawData;

		if( inStream.GetSize() > 0 )
		{
			inStream.SeekBegin();
			if( inStream.ReadBytes( rawData, (size_t)inStream.GetSize() ) != AssetStream::ReadResult::Success )
			{
				std::cout << "[TestAsset] Error: Failed to stream.. (AssetStream read error)\n";
			}
		}

		return std::shared_ptr< Asset >( new TestAsset( String( rawData, StringEncoding::ASCII ) ) );
	}

}