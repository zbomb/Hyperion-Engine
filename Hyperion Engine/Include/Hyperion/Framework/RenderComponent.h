/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/RenderComponent.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/Component.h"


namespace Hyperion
{


	class RenderComponent : public Component
	{

	public:

		void MarkDirty();

		void MarkStale();
		
		void AddToRenderer();

		void RemoveFromRenderer();


	};

}