/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DynamicShadowState.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{
	constexpr uint8 RENDERER_MIN_DYNAMIC_SHADOW_QUALITY			= 8;
	constexpr uint8 RENDERER_DYNAMIC_SHADOW_FRUSTUM_INDEX_NONE	= 255;
	constexpr uint8 RENDERER_DYNAMIC_SHADOW_POOL_INDEX_NONE		= 255;


	struct DynamicShadowAssignment
	{
		uint32 lightIndex;
		uint8 pointFrustumIndex;
		uint8 shadowMapPoolIndex;
		uint8 shadowMapQuality;

		std::vector< uint32 > meshIndexList;

		DynamicShadowAssignment()
			: lightIndex( 0 ), pointFrustumIndex( RENDERER_DYNAMIC_SHADOW_FRUSTUM_INDEX_NONE ), shadowMapPoolIndex( RENDERER_DYNAMIC_SHADOW_POOL_INDEX_NONE ),
			shadowMapQuality( RENDERER_MIN_DYNAMIC_SHADOW_QUALITY ), meshIndexList()
		{}

	};

	struct DynamicShadowState
	{
		std::vector< DynamicShadowAssignment > shadowAssignments;
	};

}