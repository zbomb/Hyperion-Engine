/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/DataTypes/ConcurrentQueue.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"

#include <mutex>
#include <atomic>

namespace Hyperion
{
	/*
		Concurrent Queue
		* This is a container that is thread-safe on all operations
		* One thread is able to push values into the queue, while another pops values from the queue, usually without blocking either thread
		* There are two locks, a front and a back lock. Only one thread can push at a time, and only one can pop at a time
		* When there is a thread pushing to the queue, the queue is considered 'back locked' meaning no other push operations can happen during this time on other threads
		* Likewise, when a thread is popping from the queue, the queue is considered 'front locked', meaning no pop operations can happen during this time on other threads
		* Sometimes though, a push or a pop requires both ends of the queue to be locked, when the element count is 0 or 1.
		* With a front lock (popping), both locks are needed when there are 1 or 0 elements in the queue
		* With a back lock (pushing), both locks are needed when the queue is empty
	*/
	template< typename _Ty >
	class ConcurrentQueue
	{

	private:

		struct Node
		{
			_Ty value;
			Node* next;
		};

		std::mutex front_mutex;
		std::mutex back_mutex;

		Node* front;
		Node* back;

		std::atomic< size_t > elem_count;
		std::atomic< size_t > full_lock_count;

		bool _pop_needs_full_lock()
		{
			return elem_count <= 1;
		}

		void _pop_half_lock()
		{
			// Must have front mutex locked before calling this, must
			// have gotten false back from _pop_needs_full_lock while under front lock
			HYPERION_VERIFY( front != nullptr && front->next != nullptr, "Concurrent queue state is invalid for this call!" );

			Node* old_front = front;
			front = front->next;
			delete old_front;
			elem_count--;
		}

		void _pop_full_lock()
		{
			// Before calling this, a lock on both mutex must be aquired
			// Outside of that, all other possibile state conditions are checked
			full_lock_count++;

			// Check for an empty queue
			if( front == nullptr )
			{
				// If front is null, back should be too
				HYPERION_VERIFY( back == nullptr, "Concurrent queue state is invalid!" );
				elem_count = 0;
			}
			// Check for single element queue
			else if( front == back )
			{
				// Use a recursive function to ensure the trailing node is deleted as well
				_destroy_node_recursive( front );
				front = nullptr;
				back = nullptr;
				elem_count = 0;
				return;
			}
			// More than 1 element
			else
			{
				// If there are more than one elements, this shouldnt be null either
				HYPERION_VERIFY( front->next != nullptr, "Concurrent queue state is invalid" );

				Node* old_front = front;
				front = front->next;
				delete old_front;
				elem_count--;
			}
		}

		bool _push_needs_full_lock()
		{
			// To bring the element count down to 0, there has to be a full lock, so if this returns false,
			// we can be sure that the queue wont be emptied, since we hold a half lock when this is called
			return elem_count <= 0 || back == nullptr;
		}

		void _push_half_lock_copy( const _Ty& in )
		{
			// Must have back mutex locked before calling this, and also gotten a false value from
			// _push_needs_full_lock() while under the back end lock
			HYPERION_VERIFY( back != nullptr && back->next != nullptr, "Concurrent queue state is invalid for this call" );

			Node* next = back->next;
			next->value = in;
			next->next = new Node();
			next->next->next = nullptr;
			back = next;
			elem_count++;
		}

		void _push_full_lock_copy( const _Ty& in )
		{
			// Must have both mutex locked before calling this
			full_lock_count++;

			// Check if empty
			if( back == nullptr )
			{
				// Both front and back need to be null when empty
				HYPERION_VERIFY( front == nullptr, "Concurrent queue state is invalid" );

				Node* ins_node = new Node();
				ins_node->value = in;
				ins_node->next = new Node();
				ins_node->next->next = nullptr;
				front = ins_node;
				back = ins_node;
				elem_count = 1;
			}
			// One or more elements
			else
			{
				// Ensure the trailing node is valid
				HYPERION_VERIFY( back->next != nullptr, "Concurrent queue state is invalid" );

				Node* next = back->next;
				next->value = in;
				next->next = new Node();
				next->next->next = nullptr;
				back = next;
				elem_count++;
			}
		}

		void _push_half_lock_move( _Ty&& in )
		{
			// Must have back mutex locked before calling this, and also gotten a false value from
			// _push_needs_full_lock() while under the back end lock
			HYPERION_VERIFY( back != nullptr && back->next != nullptr, "Concurrent queue state is invalid for this call" );

			Node* next = back->next;
			next->value = std::move( in );
			next->next = new Node();
			next->next->next = nullptr;
			back = next;
			elem_count++;
		}

		void _push_full_lock_move( _Ty&& in )
		{
			// Must have both mutex locked before calling this
			full_lock_count++;

			// Check if empty
			if( back == nullptr )
			{
				// Both front and back need to be null when empty
				HYPERION_VERIFY( front == nullptr, "Concurrent queue state is invalid" );

				Node* ins_node = new Node();
				ins_node->value = std::move( in );
				ins_node->next = new Node();
				ins_node->next->next = nullptr;
				front = ins_node;
				back = ins_node;
				elem_count = 1;
			}
			// One or more elements
			else
			{
				// Ensure the trailing node is valid
				HYPERION_VERIFY( back->next != nullptr, "Concurrent queue state is invalid" );

				Node* next = back->next;
				next->value = std::move( in );
				next->next = new Node();
				next->next->next = nullptr;
				back = next;
				elem_count++;
			}
		}

		void _destroy_node_recursive( Node* t )
		{
			full_lock_count++;

			// Not actually a recursive function to avoid stack overflows
			Node* next = t;
			while( next != nullptr )
			{
				Node* last = next;
				next = next->next;
				delete last;
			}
		}

		size_t _perform_full_lock_count()
		{
			full_lock_count++;

			if( front == nullptr )
			{
				HYPERION_VERIFY( back == nullptr, "Concurrent queue state is invalid" );
				return 0;
			}

			// Were starting at zero instead of one because we will have a trailing
			// node, so our count would be 1 higher than it actually is
			size_t count = 0;
			Node* this_node = front;
			while( this_node->next != nullptr )
			{
				this_node = this_node->next;
				count++;
			}

			return count;
		}

		void _empty_full_lock()
		{
			full_lock_count++;

			// We should have a full lock before calling this function
			if( front != nullptr )
			{
				_destroy_node_recursive( front );
				
				front = nullptr;
				back = nullptr;
				elem_count = 0;
			}
			else
			{
				HYPERION_VERIFY( back == nullptr, "Concurrent state is invalid" );
				// If front == nullptr, so should the back ptr as well
			}
		}

		void _copy_from_other( const ConcurrentQueue& Other )
		{
			// Ensure this is empty
			_empty_full_lock();

			// The locks should be in place before calling this!
			// Check if the other queue is empty
			if( Other.front == nullptr )
			{
				HYPERION_VERIFY( Other.back == nullptr, "Concurrent queue state is invalid" );
				
				elem_count	= 0;
				front		= nullptr;
				back		= nullptr;
			}
			else
			{
				// First, copy over the first node
				Node* new_first = new Node();
				new_first->next = new Node();
				new_first->next->next = nullptr;
				new_first->value = Other.front->value;

				front		= new_first;
				back		= new_first;
				elem_count	= 1;

				// Copy the rest of the nodes into the queue
				Node* target_node = Other.front->next;
				while( target_node != nullptr && target_node->next != nullptr )
				{
					HYPERION_VERIFY( back->next != nullptr, "Concurrent queue state is invalid" );

					back->next->value = target_node->value;
					back->next->next = new Node();
					back->next->next->next = nullptr;

					back = back->next;
					elem_count++;

					target_node = target_node->next;
				}
			}
		}

		void _move_from_other( ConcurrentQueue&& Other )
		{
			// Ensure this is empty
			_empty_full_lock();

			// Copy in the target queues pointers
			front				= Other.front;
			back				= Other.back;
			elem_count			= Other.elem_count.load();
			full_lock_count		= Other.full_lock_count.load();

			// Set other queue's pointers to null, were now responsible for the memory
			Other.front				= nullptr;
			Other.back				= nullptr;
			Other.elem_count		= 0;
			Other.full_lock_count	= 0;
		}



	public:

		/*
			Default Constructor
		*/
		ConcurrentQueue()
			: front( nullptr ),
			back( nullptr ),
			elem_count( 0 ),
			full_lock_count( 0 )
		{
		}

		/*
			Copy Constructor
		*/
		ConcurrentQueue( ConcurrentQueue& Other )
			: ConcurrentQueue()
		{
			// We need a full lock on queue were constructing from
			std::lock_guard< std::mutex > front_lock( Other.front_mutex );
			std::lock_guard< std::mutex > back_lock( Other.back_mutex );

			_copy_from_other( Other );
			Other.full_lock_count++;
		}

		/*
			Move Constructor
		*/
		ConcurrentQueue( ConcurrentQueue&& Other ) noexcept
			: ConcurrentQueue()
		{
			std::lock_guard< std::mutex > front_lock( Other.front_mutex );
			std::lock_guard< std::mutex > back_lock( Other.back_mutex );

			_move_from_other( std::move( Other ) );
		}

		/*
			Copy Assignment
			* Try to not use this, it requires a full lock on both queues, and these concurrent structures
			  really shouldnt be copied around much anyway
		*/
		ConcurrentQueue& operator=( ConcurrentQueue& Other )
		{
			// We need a full lock on both queues
			std::lock_guard< std::mutex > this_front_lock( front_mutex );
			std::lock_guard< std::mutex > this_back_lock( back_mutex );
			std::lock_guard< std::mutex > other_front_lock( Other.front_mutex );
			std::lock_guard< std::mutex > other_back_lock( Other.back_mutex );

			_copy_from_other( Other );
			Other.full_lock_count++;

			return *this;
		}

		/*
			Move Assignment
		*/
		ConcurrentQueue& operator=( ConcurrentQueue&& Other ) noexcept
		{
			// We need a full lock on both queues
			std::lock_guard< std::mutex > this_front_lock( front_mutex );
			std::lock_guard< std::mutex > this_back_lock( back_mutex );
			std::lock_guard< std::mutex > other_front_lock( Other.front_mutex );
			std::lock_guard< std::mutex > other_back_lock( Other.back_mutex );

			_move_from_other( std::move( Other ) );

			return *this;
		}

		/*
			Destructor
		*/
		~ConcurrentQueue()
		{
			std::lock_guard< std::mutex > front_lock( front_mutex );
			std::lock_guard< std::mutex > back_lock( back_mutex );
			_empty_full_lock();
		}

		/*
			ConcurrentQueue::Pop
			* Removes an element from the front of the queue
			* Follows front lock mechanics
		*/
		void Pop()
		{
			std::lock_guard< std::mutex > front_lock( front_mutex );

			if( front == nullptr )
			{
				return;
			}

			if( _pop_needs_full_lock() )
			{
				std::lock_guard< std::mutex > back_lock( back_mutex );
				_pop_full_lock();
			}
			else
			{
				_pop_half_lock();
			}
		}

		/*
			ConcurrentQueue::PopValue
			* Removes an element from the front of the queue and returns the value
			* Returns true whenever the value is valid
			* Follows front lock mechanics
		*/
		std::pair< bool, _Ty > PopValue()
		{
			std::lock_guard< std::mutex > front_lock( front_mutex );

			std::pair< bool, _Ty > out;
			out.first = false;

			if( front == nullptr )
			{
				return out;
			}

			out.second = std::move( front->value );
			out.first = true;

			if( _pop_needs_full_lock() )
			{
				std::lock_guard< std::mutex > back_lock( back_mutex );
				_pop_full_lock();
			}
			else
			{
				_pop_half_lock();
			}

			return out;
		}

		/*
			ConcurrentQueue::PeekFront
			* Returns the value from the front of the queue
			* If the value couldnt be read, the boolean will be false
			* Only ever locks the front lock.. never will lock back lock
		*/
		std::pair< bool, _Ty > PeekFront()
		{
			std::lock_guard< std::mutex > front_lock( front_mutex );

			std::pair< bool, _Ty > out;
			if( front == nullptr )
			{
				out.first = false;
			}
			else
			{
				out.first = true;
				out.second = std::move( front->value );
			}

			return out;
		}

		/*
			ConcurrentQueue::TryPop
			* Attempts to pop a value from the front of the queue
			* Fails (void return) if the needed lock(s) couldnt be aquired right away
			* Follows normal 'front' locking mechanics
		*/
		bool TryPop()
		{
			// Manually try-lock mutexes
			if( !front_mutex.try_lock() )
			{
				return false;
			}

			if( front == nullptr )
			{
				front_mutex.unlock();
				return true;
			}

			if( _pop_needs_full_lock() )
			{
				if( !back_mutex.try_lock() )
				{
					front_mutex.unlock();
					return false;
				}

				_pop_full_lock();
				back_mutex.unlock();
				front_mutex.unlock();
				return true;
			}
			else
			{
				_pop_half_lock();
				front_mutex.unlock();
				return true;
			}
		}

		/*
			ConcurrentQueue::TryPopValue
			* Attempts to pop a value from the front of the queue and return it to the caller
			* Fails (returns false) if the lock(s) couldnt be aquired
			* Follows normal 'front' locking mechanics
		*/
		std::pair< bool, _Ty > TryPopValue()
		{
			std::pair< bool, _Ty > out;
			out.first = false;

			if( !front_mutex.try_lock() )
			{
				return out;
			}

			if( front == nullptr )
			{
				front_mutex.unlock();
				return out;
			}

			if( _pop_needs_full_lock() )
			{
				if( !back_mutex.try_lock() )
				{
					front_mutex.unlock();
					return out;
				}

				out.first	= true;
				out.second	= std::move( front->value );
				_pop_full_lock();

				back_mutex.unlock();
				front_mutex.unlock();
				return out;
			}
			else
			{
				out.first	= true;
				out.second	= std::move( front->value );
				_pop_half_lock();

				front_mutex.unlock();
				return out;
			}
		}

		/*
			ConcurrentQueue::Push [Copy Semantics]
			* Pushes a copy of the value to the back of the queue
			* Follows normal 'back' locking mechanics
		*/
		void Push( const _Ty& in )
		{
			std::lock_guard< std::mutex > back_lock( back_mutex );

			if( _push_needs_full_lock() )
			{
				std::lock_guard< std::mutex > front_lock( front_mutex );
				_push_full_lock_copy( in );
			}
			else
			{
				_push_half_lock_copy( in );
			}
		}

		/*
			ConcurrentQueue::Push [Move Semantics]
			* Pushes a value to the back of the queue using move semantics
			* Follows normal 'back' locking mechanics
		*/
		void Push( _Ty&& in )
		{
			std::lock_guard< std::mutex > back_lock( back_mutex );
			if( _push_needs_full_lock() )
			{
				std::lock_guard< std::mutex > front_lock( front_mutex );
				_push_full_lock_move( std::move( in ) );
			}
			else
			{
				_push_half_lock_move( std::move( in ) );
			}
		}

		/*
			ConcurrentQueue::TryPush [Copy Semantics]
			* Attempts to copy the value to the back of the queue
			* Fails if the lock(s) couldnt be aquired
			* Follows normal 'back' locking mechanics
		*/
		bool TryPush( const _Ty& in )
		{
			if( !back_mutex.try_lock() )
			{
				return false;
			}

			if( _push_needs_full_lock() )
			{
				if( !front_mutex.try_lock() )
				{
					back_mutex.unlock();
					return false;
				}

				_push_full_lock_copy( in );
				front_mutex.unlock();
				back_mutex.unlock();

				return true;
			}
			else
			{
				_push_half_lock_copy( in );
				back_mutex.unlock();

				return true;
			}
		}

		/*
			ConcurrentQueue::TryPush [Move Semantics]
			* Attempts to move the value into the back of the queue
			* Fails if the lock(s) couldnt be aquired
			* Follows normal 'back' locking semantics
		*/
		bool TryPush( _Ty&& in )
		{
			if( !back_mutex.try_lock() )
			{
				return false;
			}

			if( _push_needs_full_lock() )
			{
				if( !front_mutex.try_lock() )
				{
					back_mutex.unlock();
					return false;
				}

				_push_full_lock_move( std::move( in ) );
				front_mutex.unlock();
				back_mutex.unlock();

				return true;
			}
			else
			{
				_push_half_lock_move( std::move( in ) );
				back_mutex.unlock();

				return true;
			}
		}

		/*
			ConcurrentQueue::Count
			* Determines how many elements are in the queue
			* Can optionally lock the queue to perform a more percise count
		*/
		size_t Count( bool bLock = false )
		{
			if( bLock )
			{
				std::lock_guard< std::mutex > front_lock( front_mutex );
				std::lock_guard< std::mutex > back_lock( back_mutex );
			
				return _perform_full_lock_count();
			}
			else
			{
				return elem_count;
			}
		}

		/*
			ConcurrentQueue::IsEmpty
			* Checks if the queue is empty, with optional locking
		*/
		bool IsEmpty( bool bShouldLock = false )
		{
			if( bShouldLock )
			{
				std::lock_guard< std::mutex > front_lock( front_mutex );
				return front == nullptr;
			}
			else
			{
				return elem_count <= 0;
			}
		}

		/*
			ConcurrentQueue::Clear
			* Empties the whole queue
			* Locks both ends.. might block!
		*/
		void Clear()
		{
			std::lock_guard< std::mutex > front_lock( front_mutex );
			std::lock_guard< std::mutex > back_lock( back_mutex );

			_empty_full_lock();
		}

		/*
			ConcurrentQueue::TryClear
			* Attempts to empty the queue
			* If the locks cant be aquired it returns false
		*/
		bool TryClear()
		{
			if( !front_mutex.try_lock() )
			{
				return false;
			}

			if( !back_mutex.try_lock() )
			{
				front_mutex.unlock();
				return false;
			}

			_empty_full_lock();
			back_mutex.unlock();
			front_mutex.unlock();

			return true;
		}

		/*
			ConcurrentQueue::GetFullLockCount
			* Debug function
			* Gets the counter of how many times a full lock was required
		*/
		size_t GetFullLockCount() const
		{
			return full_lock_count;
		}


		



	};

}