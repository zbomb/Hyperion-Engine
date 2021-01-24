/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/ViewClusters.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{

	class RViewClusters
	{

	public:

		float ClusterSizeX = 0.f;
		float ClusterSizeY = 0.f;

		virtual ~RViewClusters() {}
		virtual void Shutdown() = 0;
		virtual bool IsValid() const = 0;
		virtual void MarkDirty() = 0;
		virtual bool IsDirty() const = 0;
		virtual void MarkClean() = 0;

	};

}