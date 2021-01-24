/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Proxy/ProxyPrimitive.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Renderer/Proxy/ProxyBase.h"
#include "Hyperion/Library/Geometry.h"


namespace Hyperion
{

	/*
	*	Forward Declarations
	*/
	class RBuffer;
	class RMaterial;


	class ProxyPrimitive : public ProxyBase
	{

	public:

		ProxyPrimitive() = delete;
		ProxyPrimitive( uint32 inIdentifier )
			: ProxyBase( inIdentifier )
		{}

		virtual ~ProxyPrimitive() {}

		virtual Transform GetWorldTransform() const = 0;
		virtual uint8 GetActiveLOD() const = 0;
		virtual uint8 GetLODCount() const = 0;
		virtual std::map< uint8, std::shared_ptr< RMaterial > > GetMaterials() const = 0;
		virtual bool GetLODResources( uint8 inLOD, std::vector< std::tuple< std::shared_ptr< RBuffer >, std::shared_ptr< RBuffer >, std::shared_ptr< RMaterial > > >& outData ) = 0;
		virtual AABB GetAABB() const = 0;
		virtual BoundingSphere GetBoundingSphere() const = 0;


	};

}