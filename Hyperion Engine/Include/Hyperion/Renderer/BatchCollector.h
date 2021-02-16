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

	struct MeshBatch
	{
		std::vector< Matrix > InstanceTransforms;
		std::shared_ptr< RMaterial > Material;
	};

	struct MeshBatchGroup
	{
		std::shared_ptr< RBuffer > IndexBuffer;
		std::shared_ptr< RBuffer > VertexBuffer;
		std::map< uint32, MeshBatch > Batches;
	};

	/*
	*	PrimitiveCollector
	*	* Collects a list of meshes from the scene, which can then be iterated through and rendered
	*	* Used by the RenderPipeline
	*/
	class BatchCollector
	{

	public:

		BatchCollector()
		{

		}


		~BatchCollector()
		{
			Clear();
		}

		std::map< uint32, MeshBatchGroup > m_OpaqueGroups;
		std::map< uint32, MeshBatchGroup > m_TranslucentGroups;

		void CollectOpaque( const Matrix& worldMatrix, const std::shared_ptr< RMaterial >& inMaterial, const std::shared_ptr< RBuffer >& inIndex, const std::shared_ptr< RBuffer >& inVertex )
		{
			if( inMaterial == nullptr || inIndex == nullptr || inVertex == nullptr ) { return; }

			auto geomId			= inIndex->GetAssetIdentifier();
			auto& batchGroup	= m_OpaqueGroups[ geomId ];

			if( batchGroup.IndexBuffer == nullptr )		{ batchGroup.IndexBuffer = inIndex; }
			if( batchGroup.VertexBuffer == nullptr )	{ batchGroup.VertexBuffer = inVertex; }

			auto& materialGroup = batchGroup.Batches[ inMaterial->GetIdentifier() ];

			materialGroup.InstanceTransforms.push_back( worldMatrix );
			if( materialGroup.Material == nullptr ) { materialGroup.Material = inMaterial; }
		}

		void CollectTranslucent( const Matrix& worldMatrix, const std::shared_ptr< RMaterial >& inMaterial, const std::shared_ptr< RBuffer >& inIndex, const std::shared_ptr< RBuffer >& inVertex )
		{
			if( inMaterial == nullptr || inIndex == nullptr || inVertex == nullptr ) { return; }

			auto geomId			= inIndex->GetAssetIdentifier();
			auto& batchGroup	= m_TranslucentGroups[ geomId ];

			if( batchGroup.IndexBuffer == nullptr )		{ batchGroup.IndexBuffer = inIndex; }
			if( batchGroup.VertexBuffer == nullptr )	{ batchGroup.VertexBuffer = inVertex; }

			auto& materialGroup = batchGroup.Batches[ inMaterial->GetIdentifier() ];

			materialGroup.InstanceTransforms.push_back( worldMatrix );
			if( materialGroup.Material == nullptr ) { materialGroup.Material = inMaterial; }
		}


		void Clear()
		{
			m_OpaqueGroups.clear();
			m_TranslucentGroups.clear();
		}

	};

}