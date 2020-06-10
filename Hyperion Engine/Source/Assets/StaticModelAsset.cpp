/*==================================================================================================
	Hyperion Engine
	Source/Assets/StaticModelAsset.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Assets/StaticModelAsset.h"
#include "Hyperion/Tools/HSMReader.h"


namespace Hyperion
{

	std::map< uint32, std::weak_ptr< StaticModelAsset > > StaticModelAssetCache::m_Cache;
	std::mutex StaticModelAssetCache::m_CacheMutex;


	/*----------------------------------------------------------------------------------------------------------
		StaticModelAssetCache Class
	------------------------------------------------------------------------------------------------------------*/
	std::weak_ptr< StaticModelAsset > StaticModelAssetCache::Get( uint32 inIdentifier )
	{
		std::lock_guard< std::mutex > lock( m_CacheMutex );

		auto entry = m_Cache.find( inIdentifier );
		if( entry == m_Cache.end() )
		{
			return std::weak_ptr< StaticModelAsset >();
		}

		if( entry->second.expired() )
		{
			m_Cache.erase( entry );
			return std::weak_ptr< StaticModelAsset >();
		}
		else
		{
			return entry->second;
		}
	}


	void StaticModelAssetCache::Store( uint32 inIdentifier, const std::shared_ptr< StaticModelAsset >& inPtr )
	{
		std::lock_guard< std::mutex > lock( m_CacheMutex );

		m_Cache.emplace( inIdentifier, std::weak_ptr< StaticModelAsset >( inPtr ) );
	}


	/*----------------------------------------------------------------------------------------------------------
		StaticModelAssetLoader Class
	------------------------------------------------------------------------------------------------------------*/
	std::shared_ptr< StaticModelAsset > StaticModelAssetLoader::Load( uint32 inIdentifier, std::unique_ptr< IFile >& inFile )
	{
		if( inIdentifier == ASSET_INVALID || !inFile || !inFile->IsValid() )
		{
			Console::WriteLine( "[ERROR] StaticModelAssetLoader: Failed to load asset.. identifier and/or file was invalid" );
			return nullptr;
		}

		// First, we want to use the HSM loader to create a structure we can use to fill out our asset
		auto newAsset = std::shared_ptr< StaticModelAsset >( new StaticModelAsset( inFile->GetPath(), inIdentifier ) );
		if( HSMReader::ReadFile( *inFile, newAsset ) != HSMReader::Success )
		{
			return nullptr;
		}

		return newAsset;
	}


	bool StaticModelAssetLoader::IsValidFile( const FilePath& inPath )
	{
		return inPath.Extension().ToLower().Equals( ".hsm" );
	}

	/*----------------------------------------------------------------------------------------------------------
		StaticModelAsset Class
	------------------------------------------------------------------------------------------------------------*/

	StaticModelAsset::StaticModelAsset( const FilePath& inPath, uint32 inIdentifier )
		: AssetBase( inPath, inIdentifier )
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

}