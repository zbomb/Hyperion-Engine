/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resources/RMesh.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resources/RBuffer.h"
#include "Hyperion/Assets/StaticModelAsset.h"


namespace Hyperion
{
	/*
	*	RMeshBatch Structure
	*/
	struct RMeshBatch
	{
		uint8 materialSlot;
		std::shared_ptr< RBuffer > vertexBuffer;
		std::shared_ptr< RBuffer > indexBuffer;

		RMeshBatch( uint8 inMaterial, const std::shared_ptr< RBuffer >& inVertBuffer = nullptr, const std::shared_ptr< RBuffer >& inIndexBuffer = nullptr )
			: materialSlot( inMaterial ), vertexBuffer( inVertBuffer ), indexBuffer( inIndexBuffer )
		{}
	};

	/*
	*	MeshLOD Structure
	*/
	struct RMeshLOD
	{
		bool bCached;
		std::vector< RMeshBatch > batchList;
	};

	/*
	*	MeshData Structure
	*/
	struct RMeshData
	{

	protected:

		std::vector< std::shared_ptr< RMeshLOD > > m_LODs;

	public:

		RMeshData( const std::shared_ptr< StaticModelAsset >& inAsset )
		{
			// Verify that the asset is valid..
			HYPERION_VERIFY( inAsset, "[Renderer] StaticModelAsset was null?" );

			// And loop through and fill out the data structure
			for( auto it = inAsset->LODBegin(); it != inAsset->LODEnd(); it++ )
			{
				auto newLOD = std::make_shared< RMeshLOD >();
				for( auto ot = it->SubObjects.begin(); ot != it->SubObjects.end(); ot++ )
				{
					newLOD->batchList.emplace_back( RMeshBatch( ot->MaterialIndex ) );
				}

				newLOD->bCached = false;
				m_LODs.push_back( newLOD );
			}
		}


		void Shutdown()
		{
			m_LODs.clear();
		}


		inline uint8 GetLODCount() const { return (uint8)m_LODs.size(); }
		inline std::shared_ptr< RMeshLOD > GetLOD( uint8 inLOD ) const { return ( m_LODs.size() > (uint32) inLOD ) ? m_LODs.at( (uint32) inLOD ) : nullptr; }
		

		bool IsLODCached( uint8 inLOD ) const
		{
			auto lod = GetLOD( inLOD );
			if( lod )
			{
				return lod->bCached;
			}

			return false;
		}


		bool IsFullyCached() const
		{
			// Were returning false when theres no LODs
			if( m_LODs.empty() ) { return false; }

			for( uint8 i = 0; i < m_LODs.size(); i++ )
			{
				if( !m_LODs[ i ] || !m_LODs[ i ]->bCached )
				{
					return false;
				}
			}

			return true;
		}


		bool IsPartiallyCached() const
		{
			for( uint8 i = 0; i < m_LODs.size(); i++ )
			{
				if( m_LODs[ i ] && m_LODs[ i ]->bCached )
				{
					return true;
				}
			}

			return false;
		}
	};

	/*
	*	Mesh Class
	*/
	class RMesh
	{

	protected:

		uint32 m_Identifier;
		std::shared_ptr< RMeshData > m_Data;

		RMesh( uint32 inIdentifier, const std::shared_ptr< RMeshData >& inData )
			: m_Identifier( inIdentifier ), m_Data( inData )
		{}

		RMesh( uint32 inIdentifier )
			: m_Identifier( inIdentifier ), m_Data( nullptr )
		{}

	public:

		RMesh() = delete;

		~RMesh()
		{
			Shutdown();
		}

		void Shutdown()
		{
			m_Data.reset();
		}

		bool IsValid() const
		{
			return m_Data != nullptr;
		}

		inline uint32 GetIdentifier() const { return m_Identifier; }
		inline std::shared_ptr< RMeshData > GetData() const { return m_Data; }

		friend class ResourceManager;
	};

}