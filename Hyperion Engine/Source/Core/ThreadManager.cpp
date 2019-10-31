/*==================================================================================================
	Hyperion Engine
	Source/Core/ThreadManager.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/ThreadManager.h"
#include "Hyperion/Core/Engine.h"
#include <iostream>

namespace Hyperion
{


	void ThreadManager::Initialize()
	{

	}

	void ThreadManager::Shutdown()
	{
		StopThreads();
	}


	bool ThreadManager::StopThreads()
	{
		std::cout << "[DEBUG] ThreadManager: Shutting down all threads...\n";

		if( m_EngineThread )
			m_EngineThread->Stop();

		if( m_RenderThread )
			m_RenderThread->Stop();

		if( m_PhysicsThread )
			m_PhysicsThread->Stop();

		if( m_BackgroundThread )
			m_BackgroundThread->Stop();

		return true;
	}


	std::shared_ptr< Thread > ThreadManager::CreateThread( const std::string& Identifier, std::function< void() > tickFunc, std::function< void() > initFunc /* = nullptr */, std::function< void() > shutdownFunc /* = nullptr */ )
	{
		if( !tickFunc )
		{
			std::cout << "[ERROR] ThreadManager: Failed to create thread.. tick func was not supplied!\n";
			return nullptr;
		}

		auto& Eng = Engine::GetInstance();
		std::shared_ptr< Thread > Output = Eng.CreateObject< Thread >();

		Output->m_TickFunction	= tickFunc;
		Output->m_Name			= Identifier;

		if( initFunc )
			Output->m_InitFunction = initFunc;
		if( shutdownFunc )
			Output->m_ShutdownFunction = shutdownFunc;

		return Output;
	}


}