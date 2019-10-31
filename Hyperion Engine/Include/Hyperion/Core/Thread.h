/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Thread.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Object.h"
#include <thread>
#include <functional>
#include <atomic>
#include <string>
#include <mutex>
#include <queue>


namespace Hyperion
{

	class Thread : public Object
	{

	public:

		HYPERION_GROUP_OBJECT( CACHE_CORE );

		enum class Status
		{
			NotStarted,
			Running,
			Stopped
		};

	private:

		std::unique_ptr< std::thread > m_Thread;
		std::function< void( void ) > m_InitFunction;
		std::function< void( void ) > m_TickFunction;
		std::function< void( void ) > m_ShutdownFunction;
		std::string m_Name;
		Status m_Status;
		std::atomic< bool > m_bKillThread;
		std::queue< std::function< void() > > m_AsyncQueue;
		std::mutex m_AsyncQueueMutex;

		void RunThread();

	public:

		Thread();
		~Thread();

		virtual void Initialize() override;
		virtual void Shutdown() override;

		inline Status GetStatus() const { return m_Status; }
		inline std::string GetName() const { return m_Name; }

		bool Start();
		bool Stop();

		void RunTask( std::function< void() > inFunc );

		std::thread::id GetSystemIdentifier();

		friend class ThreadManager;
	};

}