/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/RenderResourceManager.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

/*
*	Headers
*/
#include "Hyperion/Hyperion.h"


namespace Hyperion
{
	/*
	*	Forward Declarations
	*/
	class IBatch;
	class StaticModelAsset;
	class MaterialAsset;
	class ILight;
	class ProxyLight;


	class RenderResourceManager
	{

	public:

		virtual ~RenderResourceManager() {}

		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;
		virtual void Clear() = 0;

		virtual std::shared_ptr< IBatch > CreateBatch( const std::shared_ptr< StaticModelAsset >& inAsset, const std::shared_ptr< MaterialAsset >& inMaterial, uint8 inLOD, uint32 inBatchIndex ) = 0;
		virtual std::shared_ptr< ILight > CreateLight( const std::shared_ptr< ProxyLight >& inLight ) = 0;
	};

}