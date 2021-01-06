/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resource/Geometry.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resource/Buffer.h"
#include "Hyperion/Assets/StaticModelAsset.h"


namespace Hyperion
{


	struct RGeometricSubObject
	{
		uint8 materialSlot;
		std::shared_ptr< RBuffer > vertexBuffer;
		std::shared_ptr< RBuffer > indexBuffer;
	};


	struct RGeometryLOD
	{
		std::vector< RGeometricSubObject > subObjects;
		bool bCached;
	};


	class RGeometryData
	{

	protected:

		std::vector< std::shared_ptr< RGeometryLOD > > m_LODs;

	public:

		RGeometryData( const std::shared_ptr< StaticModelAsset >& inAsset )
		{
			HYPERION_VERIFY( inAsset, "[RGeometryData] Invalid asset!" );

			// Fill out the structure from the data we have available.. we dont actually load any buffers
			// but instead, we just create the LODs and subobjects
			for( auto it = inAsset->LODBegin(); it != inAsset->LODEnd(); it++ )
			{
				auto lodPtr = std::make_shared< RGeometryLOD >();
				for( auto oit = it->SubObjects.begin(); oit != it->SubObjects.end(); oit++ )
				{
					RGeometricSubObject& obj = lodPtr->subObjects.emplace_back( RGeometricSubObject() );
					obj.materialSlot = oit->second.MaterialIndex;
				}

				lodPtr->bCached = false;
				m_LODs.emplace_back( lodPtr );
			}
		}

		inline uint8 GetLODCount() const				{ return (uint8) m_LODs.size(); }
		inline std::shared_ptr< RGeometryLOD > GetLOD( uint8 inLOD ) const { return( ( m_LODs.size() > inLOD ) ? m_LODs.at( inLOD ) : nullptr ); }

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

	class RGeometry
	{

	protected:

		uint32 m_Identifier;
		std::shared_ptr< RGeometryData > m_Data;

		RGeometry( uint32 inIdentifier, const std::shared_ptr< RGeometryData >& inData )
			: m_Identifier( inIdentifier ), m_Data( inData )
		{}

		RGeometry( uint32 inIdentifier )
			: RGeometry( inIdentifier, nullptr )
		{}

	public:

		RGeometry() = delete;

		bool IsValid() const { return m_Data != nullptr; }

		inline uint32 GetIdentifier() const { return m_Identifier; }
		inline std::shared_ptr< RGeometryData > GetData() const { return m_Data; }


		friend class ResourceManager;
	};

}