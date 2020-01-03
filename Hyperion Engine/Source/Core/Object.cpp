/*==================================================================================================
	Hyperion Engine
	Source/Core/Object.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/Object.h"
#include "Hyperion/Core/Engine.h"


namespace Hyperion
{
	std::map< uint32, std::shared_ptr< _ObjectState > > __objCache;
	uint32 __objIdCounter( 0 );

	void TickObjects()
	{
		for( auto It = __objCache.begin(); It != __objCache.end(); It++ )
		{
			// Validate object before ticking
			if( It->second && It->second->valid && It->second->ptr )
			{
				It->second->ptr->PerformTick();
			}
		}
	}
}
