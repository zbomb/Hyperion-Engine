/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/ThreadManager.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Object.h"
#include "Hyperion/Core/Thread.h"
#include <string>

namespace Hyperion
{

	class ThreadManager : public Object
	{

	public:

		HYPERION_GROUP_OBJECT( CACHE_CORE );

	private:

		/*
			Main Threads
		*/
		std::shared_ptr< Thread > m_EngineThread;
		std::shared_ptr< Thread > m_RenderThread;
		std::shared_ptr< Thread > m_RenderMarshalThread;
		std::shared_ptr< Thread > m_PhysicsThread;
		std::shared_ptr< Thread > m_BackgroundThread;

		bool StopThreads();

	public:

		/*
			Thread Getters
		*/
		inline std::weak_ptr< Thread > GetEngineThread()		{ return m_EngineThread ?			std::weak_ptr< Thread >( m_EngineThread ) :			std::weak_ptr< Thread >();; }
		inline std::weak_ptr< Thread > GetRenderThread()		{ return m_RenderThread ?			std::weak_ptr< Thread >( m_RenderThread ) :			std::weak_ptr< Thread >();; }
		inline std::weak_ptr< Thread > GetPhysicsThread()		{ return m_PhysicsThread ?			std::weak_ptr< Thread >( m_PhysicsThread ) :		std::weak_ptr< Thread >();; }
		inline std::weak_ptr< Thread > GetBackgroundThread()	{ return m_BackgroundThread ?		std::weak_ptr< Thread >( m_BackgroundThread ) :		std::weak_ptr< Thread >();; }
		inline std::weak_ptr< Thread > GetRenderMarshalThread()	{ return m_RenderMarshalThread ?	std::weak_ptr< Thread >( m_RenderMarshalThread ) :	std::weak_ptr< Thread >(); }

		void Initialize() override;
		void Shutdown() override;

		

		/*
			Thread Creation
		*/
		std::shared_ptr< Thread > CreateThread( const std::string& Identifier, std::function< void() > tickFunc, std::function< void() > initFunc = nullptr, std::function< void() > shutdownFunc = nullptr );

		friend class Engine;

	};


}