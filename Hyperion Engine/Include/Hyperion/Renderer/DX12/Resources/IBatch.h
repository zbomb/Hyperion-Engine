/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resources/IBatch.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{

	class IBatch
	{

	public:



	};


	struct BatchGroup
	{

	public:

		std::vector< std::shared_ptr< IBatch > > batchList;
		Matrix worldMatrix;

	};

}