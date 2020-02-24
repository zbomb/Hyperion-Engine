/*==================================================================================================
	Hyperion Engine
	Source/Core/DiskManager.cpp
	© 2019, Zachary Berry
==================================================================================================*/


#include "Hyperion/Core/DiskManager.h"
#include "Hyperion/Core/VirtualFileSystem.h"
#include "Hyperion/Core/Stream.h"


namespace Hyperion
{
	static ConsoleVar< uint32 > g_CVar_DiskManager_ThreadCount(
		"dm_thread_count", "The number of threads being used to read files from the disk",
		2, 1, 4 ); // 2 default, 4 max

	static ConsoleVar< uint32 > g_CVar_DiskManager_ChunkSize(
		"dm_chunk_size", "The number of bytes to read in a single chunk before checking for higher priority read requests",
		128 * 1024, 16 * 1024, 1024 * 1024 );


	void DiskManager::Initialize()
	{
		HYPERION_VERIFY( m_Threads.size() == 0, "DiskManager was initialized more than once!" );

		uint32 threadCount = g_CVar_DiskManager_ThreadCount.GetValue();
		HYPERION_VERIFY( threadCount > 0 && threadCount < 5, "DiskManager failed to init! Thread count invalid?" );

		CustomThreadParameters params;
		params.AllowTasks = false;
		params.Identifier = "disk_manager_worker_";
		params.StartAutomatically = true;
		params.ThreadFunction = std::bind( &DiskManager::WorkerThreadMain, std::placeholders::_1 );

		for( uint32 i = 0; i < threadCount; i++ )
		{
			params.Identifier.append( std::to_string( i ) );

			auto newThread = ThreadManager::CreateThread( params );
			HYPERION_VERIFY( newThread, "DiskManager failed to create worker threads!" );

			m_Threads.push_back( std::move( newThread ) );
		}

		Console::WriteLine( "[State] DiskManager: Initialized with ", threadCount, " threads!" );
	}


	void DiskManager::Shutdown()
	{
		for( auto& t : m_Threads )
		{
			if( t )
			{
				t->Stop();
				t.reset();
			}
		}

		m_Threads.clear();

		Console::WriteLine( "[State] DiskManager: Shutdown!" );
	}


	void DiskManager::PerformHighPriorityRead( DiskManager::ReadRequest& inRequest )
	{
		ReadResult Result;
	}


	void DiskManager::PerformLowPriorityDiskRead( DiskManager::ReadRequest& inRequest )
	{
		ReadResult Result;
	}


	void DiskManager::PerformLowPriorityVirtualRead( DiskManager::ReadRequest& inRequest )
	{
		ReadResult Result;

		auto stream = VirtualFileSystem::StreamFile( inRequest.Target.ToString() );
		if( !stream )
		{
			Console::WriteLine( "[Warning] Failed to perform low priority virtual read! \"", inRequest.Target.ToString(), "\"" );

			Result.State = ReadResultState::DidntExist;
		}
		else
		{
			// Now, we have an offset and length, we need to calculate the end, and validate that everything looks good
			// If the length is set to 0, that means we want to use the end of the file as the end
			uint32 Start = inRequest.Offset;
			uint32 End;

			if( inRequest.Length == 0 )
			{
				End = stream->GetSize();
			}
			else
			{
				End = Start + inRequest.Length;
			}

			// Validate these offsets
			if( End > stream->GetSize() )
			{
				Console::WriteLine( "[ERROR] DiskManager: Low priority virtual read failed.. attempt to read past the end of the stream! \"", inRequest.Target.ToString(), "\"" );
				Result.State = ReadResultState::ReadFailed;
			}
			else if( End - Start <= 0 )
			{
				Console::WriteLine( "[Warning] DiskManager: Attempt to read an empty set of data from a file! \"", inRequest.Target.ToString(), "\"" );
				Result.State = ReadResultState::Success;
			}
			else
			{
				// Read in chunks until we hit the end of the set were reading
				// If there is a high priority read pending, we will stop this read to perform that first
				// High priority reads simply go in order that theyre queued
				// Only issue, we want to limit how many high priority reads we allow to interrupt this lower priority read
				stream->SeekOffset( Start );

			}
		}
	}


	void DiskManager::WorkerThreadMain( CustomThread& thisThread )
	{
		// Main thread loop, check if were supposed to be running between iterations
		while( thisThread.IsRunning() )
		{
			// First, check for high priority tasks
			auto highResult = m_HighPriorityReads.PopValue();
			if( highResult.first )
			{
				// Do the read...
				PerformHighPriorityRead( highResult.second );
			}
			else
			{
				// Check for a low priority task
				auto lowResult = m_LowPriorityReads.PopValue();
				if( lowResult.first )
				{
					if( lowResult.second.bVirtualFile )
					{
						PerformLowPriorityVirtualRead( lowResult.second );
					}
					else
					{
						PerformLowPriorityDiskRead( lowResult.second );
					}

				}
				else
				{
					// There were no reads pending, so were just going to wait for the signal, after 3 milliseconds though,
					// we will just re-iterate again and check ourself anyway....
					std::unique_lock< std::mutex > lock( m_WaitMutex );
					m_WaitCV.wait_for( lock, std::chrono::milliseconds( 3 ), [] { return m_WaitBool; } );
					m_WaitBool = false;
				}
			}
		}

	}

	bool DiskManager::ScheduleRead( const DiskManager::ReadRequest& inRequest )
	{
		// Ensure there is a bound callback
		if( !inRequest.Callback )
		{
			Console::WriteLine( "[ERROR] DiskManager: Attempt to schedule a read with an unbound callback! \"", inRequest.Target.ToString(), "\"" );
			return false;
		}

		// First we need to validate the request, specifically, ensure the file even exists
		if( inRequest.bVirtualFile &&
			!VirtualFileSystem::FileExists( inRequest.Target.ToString() ) )
		{
			Console::WriteLine( "[Warning] DiskManager: Attempt to schedule a read, for a non-existing virtual file! \"", inRequest.Target.ToString(), "\"" );
			return false;
		}
		else if( !IFileSystem::FileExists( inRequest.Target ) )
		{
			Console::WriteLine( "[Warning] DiskManager: Attempt to schedule a read, for a non-existing physical file! \"", inRequest.Target.ToString(), "\"" );
			return false;
		}

		// Now, insert this request into the proper queue
		if( inRequest.bRequireLowLatency )
		{
			m_HighPriorityReads.Push( std::move( inRequest ) );
		}
		else
		{
			m_LowPriorityReads.Push( std::move( inRequest ) );
		}

		return true;
	}

}