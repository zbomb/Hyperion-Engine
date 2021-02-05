/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/PrimitiveCollector.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resources/RBuffer.h"
#include "Hyperion/Renderer/Resources/RMaterial.h"


namespace Hyperion
{
	/*
	*	BatchInfo
	*/
	struct MeshBatch
	{
		Matrix m_WorldMatrix;
		std::shared_ptr< RBuffer > m_IndexBuffer;
		std::shared_ptr< RBuffer > m_VertexBuffer;
		std::shared_ptr< RMaterial > m_Material;

		MeshBatch() = delete;
		MeshBatch( const Matrix& inTransform, const std::shared_ptr< RBuffer >& inIndexBuffer, const std::shared_ptr< RBuffer >& inVertexBuffer, const std::shared_ptr< RMaterial >& inMaterial )
			: m_WorldMatrix( inTransform ), m_IndexBuffer( inIndexBuffer ), m_VertexBuffer( inVertexBuffer ), m_Material( inMaterial )
		{}
	};

	/*
	*	PrimitiveCollector
	*	* Collects a list of meshes from the scene, which can then be iterated through and rendered
	*	* Used by the RenderPipeline
	*/
	class BatchCollector
	{

	private:

		uint32 m_Flags;
		std::vector< MeshBatch > m_Batches;

	public:

		BatchCollector()
			: m_Flags( RENDERER_GEOMETRY_COLLECTION_FLAG_NONE )
		{

		}


		~BatchCollector()
		{
			Clear();
		}

		inline std::vector< MeshBatch >::const_iterator Begin() const { return m_Batches.begin(); }
		inline std::vector< MeshBatch >::const_iterator End() const { return m_Batches.end(); }

		// Collection Flags
		inline uint32 GetFlags() const { return m_Flags; }
		void SetFlags( uint32 inFlags ) { m_Flags = inFlags; }

		// Primitive Upload
		void CollectBatch( const MeshBatch& inBatch )
		{
			if( !inBatch.m_IndexBuffer || !inBatch.m_Material || !inBatch.m_VertexBuffer ) { return; }
			m_Batches.emplace_back( inBatch );
		}

		void CollectBatch( const Matrix& inTransform, const std::shared_ptr< RBuffer >& inIndexBuffer, 
						   const std::shared_ptr< RBuffer >& inVertexBuffer, const std::shared_ptr< RMaterial >& inMaterial )
		{
			if( !inIndexBuffer || !inVertexBuffer || !inMaterial ) { return; }
			m_Batches.emplace_back( inTransform, inIndexBuffer, inVertexBuffer, inMaterial );
		}


		void Clear()
		{
			m_Batches.clear();
			m_Flags = RENDERER_GEOMETRY_COLLECTION_FLAG_NONE;
		}

	};

}