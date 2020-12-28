/*==================================================================================================
	Hyperion Engine
	Source/Assets/StaticModelAsset.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Assets/StaticModelAsset.h"
#include "Hyperion/Tools/HSMReader.h"


using namespace std::placeholders;


namespace Hyperion
{

	/*
		Register Asset Type
	*/
	AssetType g_Asset_Type_StaticModel = AssetType(
		ASSET_TYPE_STATICMODEL, "static_model", ".hsm",
		std::bind( &StaticModelAsset::LoadFromFile, _1, _2, _3, _4, _5 )
	);


	/*----------------------------------------------------------------------------------------------------------
		StaticModelAsset Class
	------------------------------------------------------------------------------------------------------------*/

	StaticModelAsset::StaticModelAsset( const String& inPath, const FilePath& inDiskPath, uint32 inIdentifier, uint64 inOffset, uint64 inLength )
		: AssetBase( inIdentifier, inPath, inOffset, inLength ), m_DiskPath( inDiskPath )
	{

	}

	std::vector<StaticModelAssetLOD>::const_iterator StaticModelAsset::GetLOD( uint8 inIndex ) const
	{
		// Ensure this LOD is valid
		if( m_LODs.size() <= (size_t) inIndex )
		{
			return m_LODs.end();
		}
		
		auto It = m_LODs.begin();
		std::advance( It, inIndex );

		return It;
	}

	uint8 StaticModelAsset::GetNumOverrideMaterialSlots( uint8 inLOD ) const
	{
		// Get this LOD, ensure its valid
		auto It = GetLOD( inLOD );
		if( It == m_LODs.end() )
		{
			return 0;
		}

		// Return the coun tof material slots...
		return (uint8)It->MaterialSlots.size();
	}

	std::shared_ptr< MaterialAsset > StaticModelAsset::GetGlobalMaterial( uint8 inSlot ) const
	{
		auto f = m_MaterialSlots.find( inSlot );
		if( f == m_MaterialSlots.end() )
		{
			return nullptr;
		}

		return f->second;
	}

	std::map<uint8, std::shared_ptr<MaterialAsset>>::const_iterator StaticModelAsset::GetOverrideMaterialsBegin( uint8 inLOD ) const
	{
		auto lodIter = GetLOD( inLOD );
		if( lodIter == m_LODs.end() )
		{
			throw;
		}

		return lodIter->MaterialSlots.begin();
	}

	std::map<uint8, std::shared_ptr<MaterialAsset>>::const_iterator StaticModelAsset::GetOverrideMaterialsEnd( uint8 inLOD ) const
	{
		auto lodIter = GetLOD( inLOD );
		if( lodIter == m_LODs.end() )
		{
			throw;
		}

		return lodIter->MaterialSlots.end();
	}

	std::shared_ptr< MaterialAsset > StaticModelAsset::GetOverrideMaterial( uint8 inLOD, uint8 inSlot ) const
	{
		auto lodIter = GetLOD( inLOD );
		if( lodIter == m_LODs.end() )
		{
			return nullptr;
		}

		auto entry = lodIter->MaterialSlots.find( inSlot );
		if( entry == lodIter->MaterialSlots.end() )
		{
			return nullptr;
		}

		return entry->second;
	}

	uint8 StaticModelAsset::GetNumSubModels( uint8 inLOD ) const
	{
		auto lodIter = GetLOD( inLOD );
		if( lodIter == m_LODs.end() )
		{
			return 0;
		}

		return (uint8)lodIter->SubObjects.size();
	}

	std::map<uint8, StaticModelAssetSubModel>::const_iterator StaticModelAsset::GetSubModelBegin( uint8 inLOD ) const
	{
		auto lodIter = GetLOD( inLOD );
		if( lodIter == m_LODs.end() )
		{
			throw;
		}
		
		return lodIter->SubObjects.begin();
	}

	std::map<uint8, StaticModelAssetSubModel>::const_iterator StaticModelAsset::GetSubModelEnd( uint8 inLOD ) const
	{
		auto lodIter = GetLOD( inLOD );
		if( lodIter == m_LODs.end() )
		{
			throw;
		}

		return lodIter->SubObjects.end();
	}

	std::map<uint8, StaticModelAssetSubModel>::const_iterator StaticModelAsset::GetSubModel( uint8 inLOD, uint8 inIndex ) const
	{
		auto lodIter = GetLOD( inLOD );
		if( lodIter == m_LODs.end() )
		{
			throw;
		}
		
		// Check if the index falls in range
		if( lodIter->SubObjects.size() <= (size_t) inIndex )
		{
			return lodIter->SubObjects.end();
		}

		auto It = lodIter->SubObjects.begin();
		std::advance( It, inIndex );

		return It;
	}


	/*
		Loader Function
	*/
	std::shared_ptr< AssetBase > StaticModelAsset::LoadFromFile( std::unique_ptr< File >& inFile, const String& inPath, uint32 inIdentifier, uint64 inOffset, uint64 inLength )
	{
		// Validate the file
		if( inIdentifier == ASSET_INVALID || !inFile || !inFile->IsValid() )
		{
			Console::WriteLine( "[Warning] StaticModelAsset: Failed to load from file.. file was invalid (Asset ID: ", inIdentifier, ")" );
			return nullptr;
		}

		// Engage the loader

		auto newAsset = std::shared_ptr< StaticModelAsset >( new StaticModelAsset( inPath, inFile->GetPath(), inIdentifier, inOffset, inLength ) );
		if( HSMReader::ReadFile( *inFile, newAsset ) != HSMReader::Result::Success )
		{
			Console::WriteLine( "[Warning] StaticModelAsset: Failed to load from file.. loader failed!" );
			return nullptr;
		}

		return std::dynamic_pointer_cast< AssetBase >( newAsset );
	}

}