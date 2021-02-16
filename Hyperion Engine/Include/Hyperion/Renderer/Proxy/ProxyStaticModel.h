/*==================================================================================================
	Hyperion Engine
	Hyperion/Renderer/Proxy/ProxyStaticModel.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Proxy/ProxyPrimitive.h"
#include "Hyperion/Renderer/Resources/RMaterial.h"
#include "Hyperion/Library/Geometry.h"
#include "Hyperion/Renderer/Resources/RMesh.h"
#include "Hyperion/Renderer/BatchCollector.h"


namespace Hyperion
{



	class ProxyStaticModel : public ProxyPrimitive
	{

	protected:

		std::shared_ptr< RMesh > m_Model;
		std::map< uint8, std::shared_ptr< RMaterial > > m_Materials;

		std::shared_ptr< StaticModelAsset > m_ModelAsset;
		std::map< uint8, std::shared_ptr< MaterialAsset > > m_MaterialAssets;

		uint8 m_ActiveLOD;
		uint8 m_LODCount;

	public:

		ProxyStaticModel() = delete;
		ProxyStaticModel( uint32 inIdentifier );
		virtual ~ProxyStaticModel();

		void GameInit() override;
		void RenderInit() override;

		void BeginShutdown() override;
		void Shutdown() override;
		
		inline std::shared_ptr< RMesh > GetModel() const { return m_Model; }
		std::shared_ptr< RMaterial > GetMaterial( uint8 inSlot );

		uint8 GetActiveLOD() const override;
		AABB GetAABB() const override;
		BoundingSphere GetBoundingSphere() const override;
		inline uint8 GetLODCount() const override { return m_LODCount; }

		void CacheMeshes() override;
		void CollectBatches( BatchCollector& inBatch ) override;

		friend class StaticModelComponent;

	};

}