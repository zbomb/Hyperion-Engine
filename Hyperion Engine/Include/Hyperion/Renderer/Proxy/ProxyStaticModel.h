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

	public:

		ProxyStaticModel() = delete;
		ProxyStaticModel( uint32 inIdentifier );

		void GameInit() override;
		void RenderInit() override;

		void BeginShutdown() override;
		void Shutdown() override;

		inline std::shared_ptr< RGeometry > GetModel() const { return m_Model; }
		std::shared_ptr< RMaterial > GetMaterial( uint8 inSlot );

		friend class StaticModelComponent;

	};

}