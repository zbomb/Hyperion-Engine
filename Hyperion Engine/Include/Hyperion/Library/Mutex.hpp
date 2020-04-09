/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Library/Mutex.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <mutex>
#include <condition_variable>


namespace Hyperion
{
	/*
		Barrier
		* This class allows you to sync multiple threads together
		* This is used within the renderer, for syncing the Marshal and Main threads
		* Basically, you can wait until 'N' threads hit the wait function, and then all will continue
	*/
	class Barrier
	{

	private:

		std::mutex m_Mutex;
		std::condition_variable m_CV;
		size_t m_Count;
		size_t m_Threshold;
		size_t m_Generation;

	public:

		explicit Barrier( size_t inCount )
			: m_Count( inCount ),
			m_Threshold( inCount ),
			m_Generation( 0 )
		{}

		void Wait()
		{
			std::unique_lock< std::mutex > Lock{ m_Mutex };
			auto Gen = m_Generation;
			if( !--m_Count )
			{
				m_Generation++;
				m_Count = m_Threshold;
				m_CV.notify_all();
			}
			else
			{
				m_CV.wait( Lock, 
					[ this, Gen ]
					{
						return Gen != m_Generation;
					} );
			}
		}
	};

}