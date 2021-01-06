/*==================================================================================================
	Hyperion Engine
	Source/Core/InputManager.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/InputManager.h"
#include "Hyperion/Framework/Entity.h"
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
		ClearAllBindings();
		m_bShutdown = true;
	}


	void InputManager::BindKey( const KeyBindInfo& inParams, std::function< bool( const KeyPressEvent& ) > Callback, const HypPtr< Entity >& Target )
	{
		// Validate Parameters
		if( !Target || !Target->IsValid() )
		{
			Console::WriteLine( "[Warning] InputManager: Attempt to bind key to invalid entity!" );
			return;
		}
		else if( inParams.Key == Keys::NONE || inParams.Type == KeyEvent::None )
		{
			Console::WriteLine( "[Warning] InputManager: Attempt to bind invalid key event!" );
			return;
		}

		// Create key binding entry structure
		KeyMapping newMapping;
		newMapping.Target			= Target;
		newMapping.Type				= inParams.Type;
		newMapping.Key				= inParams.Key;
		newMapping.Callback			= Callback;
		newMapping.bRequireAlt		= inParams.bRequireAlt;
		newMapping.bRequireCtrl		= inParams.bRequireCtrl;
		newMapping.bRequireShift	= inParams.bRequireShift;

		// Add to mappings
		m_KeyMappings[ inParams.Key ].push_back( newMapping );
	}


	void InputManager::BindKeyRaw( const KeyBindInfo& inParams, std::function< bool( const KeyPressEvent& ) > Callback, void* Target )
	{
		// Validate Parameters
		if( !Target )
		{
			Console::WriteLine( "[Warning] InputManager: Attempt to bind key raw against a null pointer!" );
			return;
		}
		else if( inParams.Key == Keys::NONE || inParams.Type == KeyEvent::None )
		{
			Console::WriteLine( "[Warning] InputManager: Attempt to bind invalid key event (raw)!" );
			return;
		}

		// Create raw key binding entry structure
		RawKeyMapping newMapping;
		newMapping.Target			= Target;
		newMapping.Type				= inParams.Type;
		newMapping.Key				= inParams.Key;
		newMapping.Callback			= Callback;
		newMapping.bRequireAlt		= inParams.bRequireAlt;
		newMapping.bRequireCtrl		= inParams.bRequireCtrl;
		newMapping.bRequireShift	= inParams.bRequireShift;

		// Add to mappings
		m_RawKeyMappings[ inParams.Key ].push_back( newMapping );
	}


	void InputManager::BindAxis( const AxisBindInfo& inParams, std::function< bool( const InputAxisEvent& ) > Callback, const HypPtr< Entity >& Target )
	{
		// Validate parameters
		if( !Target || !Target->IsValid() )
		{
			Console::WriteLine( "[Warning] InputManager: Attempt to bind axis from an invalid entity!" );
			return;
		}
		else if( inParams.Axis == InputAxis::None )
		{
			Console::WriteLine( "[Warning] InputManager: Attempt to bind invalid axis event!" );
			return;
		}

		// Create axis binding structure
		AxisMapping newMapping;
		newMapping.Axis = inParams.Axis;
		newMapping.Mult = inParams.Mult;
		newMapping.Target = Target;
		newMapping.Callback = Callback;

		// Add to mappings
		m_AxisMappings[ inParams.Axis ].push_back( newMapping );
	}


	void InputManager::BindAxisRaw( const AxisBindInfo& inParams, std::function< bool( const InputAxisEvent& ) > Callback, void* Target )
	{
		// Validate parameters
		if( !Target )
		{
			Console::WriteLine( "[Warning] InputManager: Attempt to bind axis raw from an invalid entity!" );
			return;
		}
		else if( inParams.Axis == InputAxis::None )
		{
			Console::WriteLine( "[Warning] InputManager: Attempt to bind invalid axis raw event!" );
			return;
		}

		// Create axis binding structure
		RawAxisMapping newMapping;
		newMapping.Axis = inParams.Axis;
		newMapping.Mult = inParams.Mult;
		newMapping.Target = Target;
		newMapping.Callback = Callback;

		// Add to mappings
		m_RawAxisMappings[ inParams.Axis ].push_back( newMapping );
	}


	void InputManager::ClearBindings( const HypPtr< Entity >& Target )
	{
		if( m_bShutdown || !Target )
			return;

		// Clear any key bindings for this entity (or invalid/empty bindings)
		for( auto It = m_KeyMappings.begin(); It != m_KeyMappings.end(); It++ )
		{
			It->second.erase( std::remove_if( It->second.begin(), It->second.end(),
				[ Target ]( const KeyMapping& m ) { return !m.Target || !m.Target->IsValid() || m.Target == Target; } ), It->second.end() );
		}

		// Clear any axis bindings for this entity (or invalid/empty bindings)
		for( auto It = m_AxisMappings.begin(); It != m_AxisMappings.end(); It++ )
		{
			It->second.erase( std::remove_if( It->second.begin(), It->second.end(),
				[ Target ]( const AxisMapping& m ) { return !m.Target || !m.Target->IsValid() || m.Target == Target; } ), It->second.end() );
		}

	}


	void InputManager::ClearRawBindings( void* Target )
	{
		if( m_bShutdown || !Target )
			return;

		// Clear any key bindings for this entity (or invalid/empty bindings)
		for( auto It = m_RawKeyMappings.begin(); It != m_RawKeyMappings.end(); It++ )
		{
			It->second.erase( std::remove_if( It->second.begin(), It->second.end(),
				[ Target ]( const RawKeyMapping& m ) { return !m.Target || m.Target == Target; } ), It->second.end() );
		}

		// Clear any axis bindings for this entity (or invalid/empty bindings)
		for( auto It = m_RawAxisMappings.begin(); It != m_RawAxisMappings.end(); It++ )
		{
			It->second.erase( std::remove_if( It->second.begin(), It->second.end(),
				[ Target ]( const RawAxisMapping& m ) { return !m.Target || m.Target == Target; } ), It->second.end() );
		}
	}


	void InputManager::ClearAllBindings()
	{
		m_KeyMappings.clear();
		m_AxisMappings.clear();
		m_RawKeyMappings.clear();
		m_RawAxisMappings.clear();
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
				Console::WriteLine( "[ERROR] InputManager: Key inside of input queue has an out of range value!" );
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

			KeyPressEvent eventArgs;
			eventArgs.Key	= inKey;
			eventArgs.Type	= KeyEvent::Pressed;

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
						if( It->Callback( eventArgs ) )
							break;
					}
				}
			}

			auto rawkeyBindings = m_RawKeyMappings.find( inKey );
			if( rawkeyBindings != m_RawKeyMappings.end() )
			{
				for( auto It = rawkeyBindings->second.begin(); It != rawkeyBindings->second.end(); It++ )
				{
					// We want to make sure all requirments are met before calling the event
					// Also, we want to ensure the actual object reference is valid
					if( It->Type == KeyEvent::Pressed && It->Callback )
					{
						if( It->bRequireAlt && ( !IsKeyDown( Keys::LALT ) && !IsKeyDown( Keys::RALT ) ) )
							continue;

						if( It->bRequireCtrl && ( !IsKeyDown( Keys::LCTRL ) && !IsKeyDown( Keys::RCTRL ) ) )
							continue;

						if( It->bRequireShift && ( !IsKeyDown( Keys::LSHIFT ) && !IsKeyDown( Keys::RSHIFT ) ) )
							continue;

						if( It->Callback( eventArgs ) )
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

			KeyPressEvent eventArgs;
			eventArgs.Key	= inKey;
			eventArgs.Type	= KeyEvent::Released;

			// Loop through the key map, look for any bindings for this key
			auto keyBindings = m_KeyMappings.find( inKey );
			if( keyBindings != m_KeyMappings.end() )
			{
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

						if( It->Callback( eventArgs ) )
							break;
					}
				}
			}

			auto rawkeyBindings = m_RawKeyMappings.find( inKey );
			if( rawkeyBindings != m_RawKeyMappings.end() )
			{
				for( auto It = rawkeyBindings->second.begin(); It != rawkeyBindings->second.end(); It++ )
				{
					if( It->Type == KeyEvent::Released && It->Callback )
					{
						if( It->bRequireAlt && ( !IsKeyDown( Keys::LALT ) && !IsKeyDown( Keys::RALT ) ) )
							continue;

						if( It->bRequireCtrl && ( !IsKeyDown( Keys::LCTRL ) && !IsKeyDown( Keys::RCTRL ) ) )
							continue;

						if( It->bRequireShift && ( !IsKeyDown( Keys::LSHIFT ) && !IsKeyDown( Keys::RSHIFT ) ) )
							continue;

						if( It->Callback( eventArgs ) )
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

		auto rawaxisBindings = m_RawAxisMappings.find( Axis );
		if( rawaxisBindings != m_RawAxisMappings.end() )
		{
			InputAxisEvent Event;
			Event.Axis = Axis;

			for( auto It = axisBindings->second.begin(); It != axisBindings->second.end(); It++ )
			{
				// Ensure the object and callback are valid
				if( It->Callback )
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
			Console::WriteLine( "[ERROR] InputManager: Input key is out of range!" );
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

HYPERION_REGISTER_OBJECT_TYPE( InputManager, Object );