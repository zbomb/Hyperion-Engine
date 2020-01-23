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
	class StaticModelAsset;
	class MaterialInstance;


	class StaticModelComponent : public PrimitiveComponent
	{

	protected:

		uint32 m_ScreenSize;

		AssetRef< StaticModelAsset > m_ModelAsset;
		std::shared_ptr< MaterialInstance > m_Material;


	public:

		inline uint32 GetScreenSize() const { return m_ScreenSize; }

		inline AssetRef< StaticModelAsset > GetModelAsset() const { return m_ModelAsset; }
		inline std::shared_ptr< MaterialInstance > GetMaterial() const { return m_Material; }

	};

}