/*==================================================================================================
	Hyperion Engine
	Source/Core/InputManager.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/InputManager.h"
#include <iostream>
#include <algorithm>
#include <map>


namespace Hyperion
{

	InputManager::InputManager()
	{
		m_bShutdown = false;

		for( int i = 0; i < 256; i++ )
		{
			m_KeyStates[ i ] = false;
		}

		m_MousePosition.X = 0;
		m_MousePosition.Y = 0;

		m_bIsMouseCaptured	= false;
	}

	InputManager::~InputManager()
	{
		m_bShutdown = true;
	}


	void InputManager::BindKey( const KeyBindInfo& inParams, std::function< bool( KeyPressEvent ) > Callback, Object* Target )
	{
		// Check if the parameters are valid
		if( !Target )
		{
			std::cout << "[WARNING] InputManager: Failed to bind key.. Object target was null!\n";
			return;
		}
		else if( inParams.Key == Keys::NONE || inParams.Type == KeyEvent::None )
		{
			std::cout << "[WARNING] InputManager: Failed to bind key.. invalid KeyBindInfo!\n";
			return;
		}

		// Create key binding structure
		KeyMapping newMapping;
		newMapping.Target			= Target;
		newMapping.Type				= inParams.Type;
		newMapping.Key				= inParams.Key;
		newMapping.Callback			= Callback;
		newMapping.bRequireAlt		= inParams.bRequireAlt;
		newMapping.bRequireCtrl		= inParams.bRequireCtrl;
		newMapping.bRequireShift	= inParams.bRequireShift;

		// Add to the key map
		m_KeyMappings[ inParams.Key ].push_back( newMapping );
	}

	void InputManager::BindAxis( const AxisBindInfo& inParams, std::function< bool( InputAxisEvent ) > Callback, Object* Target )
	{
		if( !Target )
		{
			std::cout << "[WARNING] InputManager: Failed to bind axis.. Object target was null!\n";
			return;
		}
		else if( inParams.Axis == InputAxis::None )
		{
			std::cout << "[WARNING] InputManager: Failed to bind axis.. Target axis was invalid!\n";
			return;
		}

		// Create axis binding structure
		AxisMapping newMapping;
		newMapping.Axis = inParams.Axis;
		newMapping.Mult = inParams.Mult;
		newMapping.Target = Target;
		newMapping.Callback = Callback;

		m_AxisMappings[ inParams.Axis ].push_back( newMapping );
	}

	void InputManager::ClearBindings( Object* Target )
	{
		if( m_bShutdown )
			return;

		if( !Target || !Target->IsValid() )
		{
			std::cout << "[ERROR] InputManager: Failed to clear bindings, object was null!\n";
			return;
		}

		// Clear any key bindings for this object
		for( auto It = m_KeyMappings.begin(); It != m_KeyMappings.end(); It++ )
		{
			It->second.erase( std::remove_if( It->second.begin(), It->second.end(),
				[ Target ]( const KeyMapping& Mapping ) { return Mapping.Target == Target; } ), It->second.end() );
		}

		// Clear any axis bindings for this object
		for( auto It = m_AxisMappings.begin(); It != m_AxisMappings.end(); It++ )
		{
			It->second.erase( std::remove_if( It->second.begin(), It->second.end(),
				[ Target ]( const AxisMapping& Mapping ) { return Mapping.Target ==Target; } ), It->second.end() );
		}

		// Now, lets remove any empty key or axis mapping groups
		for( auto It = m_KeyMappings.begin(); It != m_KeyMappings.end(); )
		{
			if( It->second.empty() )
			{
				It = m_KeyMappings.erase( It );
			}
			else
			{
				It++;
			}
		}

		for( auto It = m_AxisMappings.begin(); It != m_AxisMappings.end(); )
		{
			if( It->second.empty() )
			{
				It = m_AxisMappings.erase( It );
			}
			else
			{
				It++;
			}
		}
	}

	void InputManager::ClearAllBindings()
	{
		m_KeyMappings.clear();
		m_AxisMappings.clear();
		m_bShutdown = true;
	}

	void InputManager::Shutdown()
	{
		// Make sure all input bindings are cleared
		ClearAllBindings();
	}

	/*
		InputManager::HandleKeyPress( Keys )
		* Puts a new key event in the queue, so the game engine can dispatch on next tick
	*/
	void InputManager::HandleKeyPress( Keys inKey )
	{
		if( inKey != Keys::NONE )
		{
			RawKeyEvent Event;
			Event.Key	= inKey;
			Event.Type	= KeyEvent::Pressed;

			std::lock_guard< std::mutex > Lock( m_KeyMutex );
			m_KeyEventQueue.push( Event );
		}
	}

	/*
		InputManager::HandleKeyRelease( Keys )
		* Puts a new key event in the queue, so the game engine can dispatch on next tick
	*/
	void InputManager::HandleKeyRelease( Keys inKey )
	{
		if( inKey != Keys::NONE )
		{
			RawKeyEvent Event;
			Event.Key	= inKey;
			Event.Type	= KeyEvent::Released;

			std::lock_guard< std::mutex > Lock( m_KeyMutex );
			m_KeyEventQueue.push( Event );
		}
	}

	/*
		InputManager::HandleAxisInput( InputAxis, int, int )
		* Puts a new axis input event in the queue, so the game engine can dispatch on next tick
	*/
	void InputManager::HandleAxisInput( InputAxis Type, int Delta, int NewValue )
	{
		// Were going to create a new axis input event, and put it in the thread-safe queue
		// so the game thread can read and dispatch the event to the various objects in game
		if( Delta != 0 && Type != InputAxis::None )
		{
			RawAxisEvent Event;
			Event.Axis		= Type;
			Event.Delta		= Delta;
			Event.NewValue	= NewValue;

			std::lock_guard< std::mutex > Lock( m_AxisMutex );
			m_AxisEventQueue.push( Event );
		}
	}

	/*
		InputManager::DispatchQueue( double )
		* Goes through the key and axis queues, and dispatches any events
	*/
	void InputManager::DispatchQueue( double Delta )
	{
		// We want to lock the queues, and copy the contents, and clear them
		std::queue< RawKeyEvent > KeyQueue;
		{
			std::lock_guard< std::mutex > Lock( m_KeyMutex );
			while( !m_KeyEventQueue.empty() )
			{
				KeyQueue.push( m_KeyEventQueue.front() );
				m_KeyEventQueue.pop();
			}
		}

		std::queue< RawAxisEvent > AxisQueue;
		{
			std::lock_guard< std::mutex > Lock( m_AxisMutex );
			while( !m_AxisEventQueue.empty() )
			{
				AxisQueue.push( m_AxisEventQueue.front() );
				m_AxisEventQueue.pop();
			}
		}

		// Now that we pulled all the data, and we no longer have to worry about threading
		// we can just loop through and dispatch the events like normal
		while( !KeyQueue.empty() )
		{
			// Pop next event
			auto Event = KeyQueue.front();
			KeyQueue.pop();

			if( Event.Key == Keys::NONE )
				continue;

			auto KeyNumber = (int) Event.Key;
			if( KeyNumber >= 256 )
			{
				std::cout << "[ERROR] InputManager: Key inside of input queue has an out of range value!\n";
				continue;
			}

			if( Event.Type == KeyEvent::Pressed )
			{
				DispatchKeyPress( Event.Key );
			}
			else if( Event.Type == KeyEvent::Released )
			{
				DispatchKeyRelease( Event.Key );
			}
		}

		// Dispatch axis events
		std::map< InputAxis, int > m_AxisValues;

		while( !AxisQueue.empty() )
		{
			auto Event = AxisQueue.front();
			AxisQueue.pop();

			if( Event.Axis == InputAxis::None )
				continue;

			// If this is the mouse, we need to do a couple special things
			if( Event.Axis == InputAxis::MouseX )
			{
				m_MousePosition.X = Event.NewValue;
			}
			else if( Event.Axis == InputAxis::MouseY )
			{
				m_MousePosition.Y = Event.NewValue;
			}

			// Accumulate axis values
			m_AxisValues[ Event.Axis ] += Event.Delta;
		}

		for( auto It = m_AxisValues.begin(); It != m_AxisValues.end(); It++ )
		{
			DispatchAxisEvent( It->first, It->second, Delta );
		}
	}


	/*
		InputManager::DispatchKeyPress( Keys )
		* INTERNAL
		* Dispatches the actual key press event to any bound objects
	*/
	void InputManager::DispatchKeyPress( Keys inKey )
	{
		auto KeyNumber = (unsigned int) inKey;

		// Check if this key is already pressed
		if( !m_KeyStates[ KeyNumber ] )
		{
			m_KeyStates[ KeyNumber ] = true;

			// Loop through the key map, look for any bindings for this key
			auto keyBindings = m_KeyMappings.find( inKey );

			if( keyBindings != m_KeyMappings.end() )
			{
				for( auto It = keyBindings->second.begin(); It != keyBindings->second.end(); It++ )
				{
					// We want to make sure all requirments are met before calling the event
					// Also, we want to ensure the actual object reference is valid
					if( It->Type == KeyEvent::Pressed && It->Target && It->Target->IsValid() && It->Callback )
					{
						if( It->bRequireAlt && ( !IsKeyDown( Keys::LALT ) && !IsKeyDown( Keys::RALT ) ) )
							continue;

						if( It->bRequireCtrl && ( !IsKeyDown( Keys::LCTRL ) && !IsKeyDown( Keys::RCTRL ) ) )
							continue;

						if( It->bRequireShift && ( !IsKeyDown( Keys::LSHIFT ) && !IsKeyDown( Keys::RSHIFT ) ) )
							continue;

						// All requirment are met, call the callback
						KeyPressEvent eventArgs;
						eventArgs.Key	= inKey;
						eventArgs.Type	= KeyEvent::Pressed;

						bool bHandled = It->Callback( eventArgs );

						// If the callback 'handled' the event, then we will stop firing it
						if( bHandled )
							break;
					}
				}
			}
		}
	}

	/*
		InputManager::DispatchKeyRelease( Keys )
		* INTERNAL
		* Dispatches the actual key press event to any bound objects
	*/
	void InputManager::DispatchKeyRelease( Keys inKey )
	{
		auto KeyNumber = (unsigned int) inKey;

		// Ensure the key is already pressed
		if( m_KeyStates[ KeyNumber ] )
		{
			m_KeyStates[ KeyNumber ] = false;

			// Loop through the key map, look for any bindings for this key
			auto keyBindings = m_KeyMappings.find( inKey );

			if( keyBindings != m_KeyMappings.end() )
			{
					KeyPressEvent eventArgs;
					eventArgs.Key	= inKey;
					eventArgs.Type	= KeyEvent::Released;

				for( auto It = keyBindings->second.begin(); It != keyBindings->second.end(); It++ )
				{
					// We want to make sure all requirments are met before calling the event
					// Also, we want to ensure the actual object reference is valid
					if( It->Type == KeyEvent::Released && It->Target && It->Target->IsValid() && It->Callback )
					{
						if( It->bRequireAlt && ( !IsKeyDown( Keys::LALT ) && !IsKeyDown( Keys::RALT ) ) )
							continue;

						if( It->bRequireCtrl && ( !IsKeyDown( Keys::LCTRL ) && !IsKeyDown( Keys::RCTRL ) ) )
							continue;

						if( It->bRequireShift && ( !IsKeyDown( Keys::LSHIFT ) && !IsKeyDown( Keys::RSHIFT ) ) )
							continue;

						// All requirment are met, call the callback
						bool bHandled = It->Callback( eventArgs );

						// If the callback 'handled' the event, then we will stop firing it
						if( bHandled )
							break;
					}
				}
			}
		}
	}

	/*
		InputManager::DispatchAxisEvent( RawAxisEvent, double )
		* INTERNAL
		* Calculates and dispatches an axis event to bound objects
	*/
	void InputManager::DispatchAxisEvent( InputAxis Axis, int Value, double Delta )
	{
		// We want to calculate the total axis input
		float Mult = 1.f;
		float AxisInput = (float)( ( (double) Value * Mult ) / ( Delta * 1000.0 ) );

		auto axisBindings = m_AxisMappings.find( Axis );
		if( axisBindings != m_AxisMappings.end() )
		{
			InputAxisEvent Event;
			Event.Axis = Axis;

			for( auto It = axisBindings->second.begin(); It != axisBindings->second.end(); It++ )
			{
				// Ensure the object and callback are valid
				if( It->Target && It->Target->IsValid() && It->Callback )
				{
					Event.Value = AxisInput * It->Mult;
					bool bHandled = It->Callback( Event );

					if( bHandled )
						break;
				}
			}
		}
	}




	bool InputManager::IsKeyDown( Keys inKey )
	{
		if( inKey == Keys::NONE )
			return false;

		auto KeyNumber = (unsigned int) inKey;
		if( KeyNumber >= 256 )
		{
			std::cout << "[ERROR] InputManager: Input key is out of range!\n";
			return false;
		}

		return m_KeyStates[ KeyNumber ];
	}



	void InputManager::CaptureMouse()
	{
		m_bIsMouseCaptured = true;

		if( m_CaptureCallback )
			m_CaptureCallback( true );
	}

	void InputManager::ReleaseMouse()
	{
		m_bIsMouseCaptured = false;

		if( m_CaptureCallback )
			m_CaptureCallback( false );
	}

	void InputManager::SetCaptureCallback( std::function< void( bool ) > Callback )
	{
		m_CaptureCallback = Callback;
	}

}