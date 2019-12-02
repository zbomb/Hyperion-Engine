/*==================================================================================================
	Hyperion Engine
	Source/Core/Object.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Object.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Core/InputManager.h"

#ifdef HYPERION_DEBUG_OBJECT
#include <iostream>
#endif

#undef min

namespace Hyperion
{
	Object::Object()
	{
		m_Identifier	= OBJECT_INVALID;
		m_TypeHash		= OBJECT_INVALID;
		m_IsValid		= false;
		b_RequiresTick	= false;
		m_LastTick		= std::chrono::time_point< std::chrono::high_resolution_clock >::min();
	}

	Object::~Object()
	{

	}

	void Object::PerformObjectInit()
	{
		Initialize();
	}

	void Object::PerformObjectShutdown()
	{
		// Stop requesting ticks
		b_RequiresTick = false;
		Shutdown();
	}

	void Object::PerformObjectTick()
	{
		if( b_RequiresTick )
		{
			// Check if this is the first tick, if so, then delta will be 0
			double Delta = 0.0;
			auto Now = std::chrono::high_resolution_clock::now();

			// Calculate the delta since last tick
			if( m_LastTick != std::chrono::time_point< std::chrono::high_resolution_clock >::min() )
			{
				std::chrono::duration< double > duration = Now - m_LastTick;
				Delta = duration.count();
			}

			// Call Tick
			Tick( Delta );

			// Update last tick
			m_LastTick = Now;
		}
	}

	void Object::Initialize()
	{

	}

	void Object::Shutdown()
	{

	}

	void Object::Tick( double Delta )
	{

	}

	void Object::BindUserInput( InputManager* Input )
	{

	}
}