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
	class BatchCollector;


	class ProxyPrimitive : public ProxyBase
	{

	public:

		// Matrix System
		bool m_bMatrixDirty;
		Matrix m_WorldMatrix;

		ProxyPrimitive() = delete;
		ProxyPrimitive( uint32 inIdentifier )
			: ProxyBase( inIdentifier ), m_bMatrixDirty( true )
		{}

		virtual ~ProxyPrimitive() {}

		virtual uint8 GetActiveLOD() const = 0;
		virtual uint8 GetLODCount() const = 0;
		virtual AABB GetAABB() const = 0;
		virtual BoundingSphere GetBoundingSphere() const = 0;
		virtual void CollectBatches( BatchCollector& inCollector ) = 0;

	};

}