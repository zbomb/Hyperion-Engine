/*==================================================================================================
	Hyperion Engine
	Hyperion/Renderer/Proxy/ProxyStaticModel.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Proxy/ProxyPrimitive.h"
#include "Hyperion/Renderer/Resource/Material.h"
#include "Hyperion/Renderer/Resource/Geometry.h"


namespace Hyperion
{



	class ProxyStaticModel : public ProxyPrimitive
	{

	protected:

		std::shared_ptr< RGeometry > m_Model;
		std::map< uint8, std::shared_ptr< RMaterial > > m_Materials;

		std::shared_ptr< StaticModelAsset > m_ModelAsset;
		std::map< uint8, std::shared_ptr< MaterialAsset > > m_MaterialAssets;

		uint8 m_ActiveLOD;
		uint8 m_LODCount;

	public:

		ProxyStaticModel() = delete;
		ProxyStaticModel( uint32 inIdentifier );

		void GameInit() override;
		void RenderInit() override;

		void BeginShutdown() override;
		void Shutdown() override;
		
		inline std::shared_ptr< RGeometry > GetModel() const { return m_Model; }
		std::shared_ptr< RMaterial > GetMaterial( uint8 inSlot );

		Transform3D GetWorldTransform() const override;
		uint8 GetActiveLOD() const override;
		AABB GetAABB() const override;
		BoundingSphere GetBoundingSphere() const override;
		inline uint8 GetLODCount() const override { return m_LODCount; }
		inline std::map< uint8, std::shared_ptr< RMaterial > > GetMaterials() const override { return m_Materials; }

		bool GetLODResources( uint8 inLOD, std::vector< std::tuple< std::shared_ptr< RBuffer >, std::shared_ptr< RBuffer >, std::shared_ptr< RMaterial > > >& outData ) override;

		friend class StaticModelComponent;

	};

}