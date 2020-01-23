/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Streaming/DynamicQualityAssetManager.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Object.h"


namespace Hyperion
{
	// Forward Declarations
	class StaticModelComponent;
	class DynamicModelComponent;
	class World;
	

	class DynamicQualityAssetManager
	{

	public:

		DynamicQualityAssetManager() = delete;

		static bool RegisterComponent( const HypPtr< StaticModelComponent >& inComponent );
		static bool RegisterComponent( const HypPtr< DynamicModelComponent >& inComponent );

		static bool DeRegisterComponent( const HypPtr< StaticModelComponent >& inComponent );
		static bool DeRegisterComponent( const HypPtr< DynamicModelComponent >& inComponent );

		static void OnWorldChanged( const HypPtr< World >& inNewWorld );

	};


}