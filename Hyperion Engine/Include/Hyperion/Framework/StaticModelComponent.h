/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/StaticModelComponent.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Framework/PrimitiveComponent.h"
#include "Hyperion/Core/Asset.h"


namespace Hyperion
{
	/*
	*	Forward Declarations
	*/
	class MaterialAsset;
	class StaticModelAsset;
	class ProxyStaticModel;



	class StaticModelComponent : public PrimitiveComponent
	{

	protected:

		// TODO: Do we need this?
		uint32 m_ScreenSize;

		std::map< uint8, std::shared_ptr< MaterialAsset > > m_MaterialSlots;
		std::shared_ptr< StaticModelAsset > m_Model;

		std::weak_ptr< ProxyStaticModel > m_Proxy;

		std::shared_ptr< ProxyPrimitive > CreateProxy() override;
		bool UpdateProxy( const std::shared_ptr< ProxyPrimitive >& inPtr ) override;

	public:

		void SetMaterial( const std::shared_ptr< MaterialAsset >& inMat, uint8 inSlot );
		void SetModel( const std::shared_ptr< StaticModelAsset >& inMdl );

		inline uint32 GetScreenSize() const { return m_ScreenSize; }
		
		inline std::shared_ptr< MaterialAsset > GetMaterial( uint8 inSlot ) const { return m_MaterialSlots.find( inSlot ) != m_MaterialSlots.end() ? m_MaterialSlots.at( inSlot ) : nullptr; }
		inline std::shared_ptr< StaticModelAsset > GetModelAsset() const { return m_Model; }

	};

}