/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/MaterialBuffer.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resources/RMaterial.h"


namespace Hyperion
{

	class RMaterialBuffer
	{

	public:

		virtual ~RMaterialBuffer() {}

		virtual void Shutdown() = 0;
		virtual bool IsValid() const = 0;

		virtual bool UploadMaterials( const std::vector< std::shared_ptr< RMaterial > >& inMaterials ) = 0;
		virtual uint32 GetMaterialCount() const = 0;

	};

}