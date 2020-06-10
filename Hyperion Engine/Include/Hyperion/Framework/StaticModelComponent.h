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


	class StaticModelComponent : public PrimitiveComponent
	{

	protected:

		// TODO: Do we need this?
		uint32 m_ScreenSize;

		

	public:

		inline uint32 GetScreenSize() const { return m_ScreenSize; }

	};

}