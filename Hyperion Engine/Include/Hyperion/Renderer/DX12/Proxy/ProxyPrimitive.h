/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Proxy/ProxyPrimitive.h
	� 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Renderer/Proxy/ProxyBase.h"
#include "Hyperion/Library/Geometry.h"
#include "Hyperion/Renderer/BatchCollector.h"



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

		// Matrix System
		bool m_bMatrixDirty;
		Matrix m_WorldMatrix;

		// Cached Batch System
		std::vector< MeshBatch > m_CachedOpaqueBatches;
		std::vector< MeshBatch > m_CachedTranslucentBatches;
		bool m_bCacheDirty;

		// Oriented Bounds
		OBB m_OrientedBounds;
		int m_CoherencyState;

		ProxyPrimitive() = delete;
		ProxyPrimitive( uint32 inIdentifier )
			: ProxyBase( inIdentifier ), m_bMatrixDirty( true ), m_bCacheDirty( true ), m_OrientedBounds(), m_CoherencyState( -1 )
		{}

		virtual ~ProxyPrimitive() {}

		virtual uint8 GetActiveLOD() const = 0;
		virtual uint8 GetLODCount() const = 0;
		virtual AABB GetAABB() const = 0;
		inline OBB GetOBB() const { return m_OrientedBounds; }
		virtual BoundingSphere GetBoundingSphere() const = 0;
		virtual void CacheBatches() = 0;
		virtual void CollectBatches( BatchCollector& inCollector ) = 0;

	};

}