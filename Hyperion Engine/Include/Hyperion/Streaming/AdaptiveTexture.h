/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Streaming/AdaptiveTexture.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Streaming/AdaptiveBase.h"


namespace Hyperion
{

	struct AdaptiveTextureLoadRequest;


	class AdaptiveTexture : public AdaptiveBase
	{

	public:

		AssetRef< TextureAsset > m_Asset;

		uint8 m_TargetLOD;
		uint8 m_ActiveLOD;
		uint8 m_PendingLOD;

		float m_LODBias;

		std::weak_ptr< AdaptiveTextureLoadRequest > m_LoadRequest;

		AdaptiveTexture()
			: m_TargetLOD( 255 ), m_LODBias( 0.5f ), m_ActiveLOD( 255 ), m_PendingLOD( 255 )
		{
		}
	};

	struct AdaptiveTextureLoadRequest
	{
		std::shared_ptr< AdaptiveTexture > m_Target;
		uint8 m_DesiredLevel;
		float m_Priority;
		std::atomic< bool > m_Valid;
	};

}