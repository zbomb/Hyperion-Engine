/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resource/Mesh.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resource/Buffer.h"
#include "Hyperion/Assets/StaticModelAsset.h"


namespace Hyperion
{

	struct RSubMesh
	{
		uint8 materialSlot;
		std::shared_ptr< RBuffer > vertexBuffer;
		std::shared_ptr< RBuffer > indexBuffer;
	};


	struct RMeshLOD
	{
		std::vector< RSubMesh > subObjects;
		bool bCached;
	};


	class RMeshData
	{

	protected:

		std::vector< std::shared_ptr< RMeshLOD > > m_LODs;

	public:

		RMeshData( const std::shared_ptr< StaticModelAsset >& inAsset )
		{
			HYPERION_VERIFY( inAsset, "[RGeometryData] Invalid asset!" );

			// Fill out the structure from the data we have available.. we dont actually load any buffers
			// but instead, we just create the LODs and subobjects
			for( auto it = inAsset->LODBegin(); it != inAsset->LODEnd(); it++ )
			{
				auto lodPtr = std::make_shared< RMeshLOD >();
				for( auto oit = it->SubObjects.begin(); oit != it->SubObjects.end(); oit++ )
				{
					RSubMesh& obj = lodPtr->subObjects.emplace_back( RSubMesh() );
					obj.materialSlot = oit->MaterialIndex;
				}

				lodPtr->bCached = false;
				m_LODs.emplace_back( lodPtr );
			}
		}

		inline uint8 GetLODCount() const				{ return (uint8) m_LODs.size(); }
		inline std::shared_ptr< RMeshLOD > GetLOD( uint8 inLOD ) const { return( ( m_LODs.size() > inLOD ) ? m_LODs.at( inLOD ) : nullptr ); }

		bool IsLODCached( uint8 inLOD ) const { return( ( m_LODs.size() > inLOD ) ? m_LODs.at( inLOD ) != nullptr && m_LODs.at( inLOD )->bCached : false ); }

		bool IsFullyCached() const
		{
			bool bResult = true;
			for( auto it = m_LODs.begin(); it != m_LODs.end(); it++ )
			{
				if( !( *it ) || !( *it )->bCached ) { bResult = false; break; }
			}

			return bResult;
		}

		bool IsPartiallyCached() const
		{
			bool bResult = false;
			for( auto it = m_LODs.begin(); it != m_LODs.end(); it++ )
			{
				if( *it && ( *it )->bCached ) { bResult = true; break; }
			}

			return bResult;
		}


	};

	class RMesh
	{

	protected:

		uint32 m_Identifier;
		std::shared_ptr< RMeshData > m_Data;

		RMesh( uint32 inIdentifier, const std::shared_ptr< RMeshData >& inData )
			: m_Identifier( inIdentifier ), m_Data( inData )
		{}

		RMesh( uint32 inIdentifier )
			: RMesh( inIdentifier, nullptr )
		{}

	public:

		RMesh() = delete;

		bool IsValid() const { return m_Data != nullptr; }

		inline uint32 GetIdentifier() const { return m_Identifier; }
		inline std::shared_ptr< RMeshData > GetData() const { return m_Data; }


		friend class ResourceManager;
	};

}