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

		TestAsset() = delete;
		TestAsset( const String & = nullptr );
		~TestAsset();

		virtual String GetAssetType() const;

		String m_Data;
	};

	template<>
	inline std::shared_ptr< Asset > AssetLoader::Load< TestAsset >( const AssetPath& Identifier, std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
	{
		std::vector< byte > rawData( Begin, End );
		return std::shared_ptr< Asset >( new TestAsset( String( rawData, StringEncoding::ASCII ) ) );
	}

	template<>
	inline std::shared_ptr< Asset > AssetLoader::Stream< TestAsset >( const AssetPath& Identifier, AssetStream& inStream )
	{
		// Were going to take this data in as a string
		std::vector< byte > rawData;

		if( inStream.GetSize() > 0 )
		{
			inStream.SeekBegin();
			if( inStream.ReadBytes( rawData, (size_t)inStream.GetSize() ) != AssetStream::ReadResult::Success )
			{
				Console::WriteLine( "[TestAsset] Error: Failed to stream.. (AssetStream read error)" );
			}
		}

		return std::shared_ptr< Asset >( new TestAsset( String( rawData, StringEncoding::ASCII ) ) );
	}

}