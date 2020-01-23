/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Async.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/ThreadManager.h"


namespace Hyperion
{ 
	class Task
	{

	public:

		template< typename T >
		static TaskHandle< T > Create( std::function< T( void ) > inFunc, std::string inTargetThread = "pool" )
		{
			return ThreadManager::CreateTask< T >( inFunc, inTargetThread );
		}

		static void WaitAll( TaskHandleBase& first )
		{
			if( !first.IsComplete() ) first.Wait();
		}

		static void WaitAll( TaskHandleBase& first, TaskHandleBase& second )
		{
			if( !first.IsComplete() ) first.Wait();
			if( !second.IsComplete() ) second.Wait();
		}

		static void WaitAll( TaskHandleBase& f, TaskHandleBase& s, TaskHandleBase& t )
		{
			if( !f.IsComplete() ) f.Wait();
			if( !s.IsComplete() ) s.Wait();
			if( !t.IsComplete() ) t.Wait();
		}

		static void WaitAll( TaskHandleBase& f, TaskHandleBase& s, TaskHandleBase& t, TaskHandleBase& fo )
		{
			if( !f.IsComplete() ) f.Wait();
			if( !s.IsComplete() ) s.Wait();
			if( !t.IsComplete() ) t.Wait();
			if( !fo.IsComplete() ) fo.Wait();
		}

		static void WaitAll( TaskHandleBase& f, TaskHandleBase& s, TaskHandleBase& t, TaskHandleBase& fo, TaskHandleBase& fi )
		{
			if( !f.IsComplete() ) f.Wait();
			if( !s.IsComplete() ) s.Wait();
			if( !t.IsComplete() ) t.Wait();
			if( !fo.IsComplete() ) fo.Wait();
			if( !fi.IsComplete() ) fi.Wait();
		}

		static void WaitAll( TaskHandleBase& f, TaskHandleBase& s, TaskHandleBase& t, TaskHandleBase& fo, TaskHandleBase& fi, TaskHandleBase& si )
		{
			if( !f.IsComplete() ) f.Wait();
			if( !s.IsComplete() ) s.Wait();
			if( !t.IsComplete() ) t.Wait();
			if( !fo.IsComplete() ) fo.Wait();
			if( !fi.IsComplete() ) fi.Wait();
			if( !si.IsComplete() ) si.Wait();
		}

		static void WaitAll( TaskHandleBase& f, TaskHandleBase& s, TaskHandleBase& t, TaskHandleBase& fo, TaskHandleBase& fi, TaskHandleBase& si, TaskHandleBase& se )
		{
			if( !f.IsComplete() ) f.Wait();
			if( !s.IsComplete() ) s.Wait();
			if( !t.IsComplete() ) t.Wait();
			if( !fo.IsComplete() ) fo.Wait();
			if( !fi.IsComplete() ) fi.Wait();
			if( !si.IsComplete() ) si.Wait();
			if( !se.IsComplete() ) se.Wait();
		}

		static void WaitAll( TaskHandleBase& f, TaskHandleBase& s, TaskHandleBase& t, TaskHandleBase& fo, TaskHandleBase& fi, TaskHandleBase& si, TaskHandleBase& se, TaskHandleBase& ei )
		{
			if( !f.IsComplete() ) f.Wait();
			if( !s.IsComplete() ) s.Wait();
			if( !t.IsComplete() ) t.Wait();
			if( !fo.IsComplete() ) fo.Wait();
			if( !fi.IsComplete() ) fi.Wait();
			if( !si.IsComplete() ) si.Wait();
			if( !se.IsComplete() ) se.Wait();
			if( !ei.IsComplete() ) ei.Wait();
		}

		/*
			If you need to call this with more than 8 tasks.. you can use this version
		*/
		static void WaitAll( std::initializer_list< TaskHandleBase* > Tasks )
		{
			for( auto& t : Tasks )
			{
				if( t && !t->IsComplete() )
					t->Wait();
			}
		}

		/*
			Or this version...
		*/
		static void WaitAll( std::vector< TaskHandleBase* > Tasks )
		{
			for( auto& t : Tasks )
			{
				if( t && !t->IsComplete() ) t->Wait();
			}
		}

		static void WaitAny( TaskHandleBase& f )
		{
			if( !f.IsComplete() ) f.Wait();
		}

		static void WaitAny( TaskHandleBase& f, TaskHandleBase& s )
		{
			WaitAny( { &f, &s } );
		}

		static void WaitAny( TaskHandleBase& f, TaskHandleBase& s, TaskHandleBase& t )
		{
			WaitAny( { &f, &s, &t } );
		}

		static void WaitAny( TaskHandleBase& f, TaskHandleBase& s, TaskHandleBase& t, TaskHandleBase& fo )
		{
			WaitAny( { &f, &s, &t, &fo } );
		}

		static void WaitAny( TaskHandleBase& f, TaskHandleBase& s, TaskHandleBase& t, TaskHandleBase& fo, TaskHandleBase& fi )
		{
			WaitAny( { &f, &s, &t, &fo, &fi } );
		}

		static void WaitAny( TaskHandleBase& f, TaskHandleBase& s, TaskHandleBase& t, TaskHandleBase& fo, TaskHandleBase& fi, TaskHandleBase& si )
		{
			WaitAny( { &f, &s, &t, &fo, &fi, &si } );
		}

		static void WaitAny( TaskHandleBase& f, TaskHandleBase& s, TaskHandleBase& t, TaskHandleBase& fo, TaskHandleBase& fi, TaskHandleBase& si, TaskHandleBase& se )
		{
			WaitAny( { &f, &s, &t, &fo, &fi, &si, &se } );
		}

		static void WaitAny( TaskHandleBase& f, TaskHandleBase& s, TaskHandleBase& t, TaskHandleBase& fo, TaskHandleBase& fi, TaskHandleBase& si, TaskHandleBase& se, TaskHandleBase& ei )
		{
			WaitAny( { &f, &s, &t, &fo, &fi, &si, &se, &ei } );
		}

		/*
			If you need to wait for more than 8 tasks.. you can use this version directly
		*/
		static void WaitAny( std::initializer_list< TaskHandleBase* > Tasks )
		{
			if( Tasks.size() > 0 )
			{
				// Create a wait token, then add it to each shared task states token list
				// This way, which ever task finishes first will trigger the token
				auto wait_token		= std::make_shared< TaskWaitToken >();
				bool bWait			= false;

				for( auto& task : Tasks )
				{
					if( task && task->AddWaitToken( wait_token ) )
					{
						bWait = true;
					}
				}

				// Check if we actually added any wait tokens before waiting
				if( bWait )
				{
					// Wait for a task to trigger our cv lock
					std::unique_lock< std::mutex > token_lock( wait_token->m );
					wait_token->cv.wait( token_lock, [ wait_token ]{ return wait_token->b; } );
				}
			}
		}

		/*
			Or this version
		*/
		static void WaitAny( const std::vector< TaskHandleBase* > Tasks )
		{
			if( Tasks.size() > 0 )
			{
				// Create a wait token, then add it to each shared task states token list
				// This way, which ever task finishes first will trigger the token
				auto wait_token		= std::make_shared< TaskWaitToken >();
				bool bWait			= false;

				for( auto& task : Tasks )
				{
					if( task && task->AddWaitToken( wait_token ) )
					{
						bWait = true;
					}
				}

				// Check if we actually added any wait tokens before waiting
				if( bWait )
				{
					// Wait for a task to trigger our cv lock
					std::unique_lock< std::mutex > token_lock( wait_token->m );
					wait_token->cv.wait( token_lock, [ wait_token ]{ return wait_token->b; } );
				}
			}
		}
	};

}