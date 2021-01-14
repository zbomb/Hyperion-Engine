/*==================================================================================================
	Hyperion Engine
	Source/Core/InputManager.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/InputManager.h"
#include "Hyperion/Core/GameInstance.h"
#include "Hyperion/Framework/LocalPlayer.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/File/FileSystem.h"
#include <iostream>
#include <algorithm>
#include <map>

/*
*	TODO: Move to RawInput API instead of mucking with windows events to read input into the game engine
*/


namespace Hyperion
{

	InputManager::InputManager()
		: m_bIsMouseCaptured( false )
	{}


	bool InputManager::BindKey( Keys inKey, const String& inCmd, bool bPress )
	{
		std::unique_lock< std::shared_mutex > lock( m_BindListMutex );

		if( inCmd.IsWhitespaceOrEmpty() )
		{
			Console::WriteLine( "[Warning] InputManager: Failed to bind key, input string was invalid" );
			return false;
		}

		if( bPress )
		{
			if( m_KeyPressBinds.find( inKey ) != m_KeyPressBinds.end() )
			{
				Console::WriteLine( "[Warning] InputManager: failed to bind \"", inCmd, "\" (press) because the key was already bound" );
				return false;
			}

			m_KeyPressBinds[ inKey ] = inCmd;
		}
		else
		{
			if( m_KeyReleaseBinds.find( inKey ) != m_KeyReleaseBinds.end() )
			{
				Console::WriteLine( "[Warning] InputManager: Failed to bind \"", inCmd, "\" (release) because the key was already bound" );
				return false;
			}

			m_KeyReleaseBinds[ inKey ] = inCmd;
		}

		return true;
	}


	bool InputManager::BindAxis( InputAxis inAxis, const String& inCmd, float inMult, bool bInvert )
	{
		std::unique_lock< std::shared_mutex > lock( m_BindListMutex );
		if( inCmd.IsWhitespaceOrEmpty() )
		{
			Console::WriteLine( "[Warning] InputManager: Failed to bind axis, input string was invalid" );
			return false;
		}

		if( m_AxisBinds.find( inAxis ) != m_AxisBinds.end() )
		{
			Console::WriteLine( "[Warning] InputManager: Failed to bind axis \"", inCmd, "\" because the axis is already bound" );
			return false;
		}

		m_AxisBinds[ inAxis ].cmd = inCmd;
		m_AxisBinds[ inAxis ].invert = bInvert;
		m_AxisBinds[ inAxis ].mult = inMult;

		return true;
	}


	bool InputManager::UnbindKey( const String& inKey )
	{
		std::unique_lock< std::shared_mutex > lock( m_BindListMutex );
		bool bFound = false;

		for( auto it = m_KeyPressBinds.begin(); it != m_KeyPressBinds.end(); )
		{
			if( it->second.Equals( inKey ) )
			{
				it = m_KeyPressBinds.erase( it );
				bFound = true;
			}
			else
			{
				it++;
			}
		}

		for( auto it = m_KeyReleaseBinds.begin(); it != m_KeyReleaseBinds.end(); )
		{
			if( it->second.Equals( inKey ) )
			{
				it = m_KeyReleaseBinds.erase( it );
				bFound = true;
			}
			else
			{
				it++;
			}
		}

		return bFound;
	}


	bool InputManager::UnbindKey( Keys inKey )
	{
		std::unique_lock< std::shared_mutex > lock( m_BindListMutex );
		auto pressVal = m_KeyPressBinds.erase( inKey );
		auto releaseVal = m_KeyReleaseBinds.erase( inKey );

		return pressVal || releaseVal;
	}


	bool InputManager::UnbindAxis( const String& inAxis )
	{
		std::unique_lock< std::shared_mutex > lock( m_BindListMutex );
		bool bFound = false;

		for( auto it = m_AxisBinds.begin(); it != m_AxisBinds.end(); )
		{
			if( it->second.cmd.Equals( inAxis ) )
			{
				it = m_AxisBinds.erase( it );
				bFound = true;
			}
		}

		return bFound;
	}


	bool InputManager::UnbindAxis( InputAxis inAxis )
	{
		std::unique_lock< std::shared_mutex > lock( m_BindListMutex );
		return m_AxisBinds.erase( inAxis );
	}


	void InputManager::ClearBindings()
	{
		std::unique_lock< std::shared_mutex > lock( m_BindListMutex );
		m_KeyPressBinds.clear();
		m_KeyReleaseBinds.clear();
		m_AxisBinds.clear();
	}


	bool InputManager::OnKeyPress( Keys inKey )
	{
		// Check if this key is already being pressed down
		if( m_KeyDownList[ inKey ] ) { return true; }

		// Get shared lock on the key binding list, and lookup binding
		std::shared_lock< std::shared_mutex > lock( m_BindListMutex );

		auto entry = m_KeyPressBinds.find( inKey );
		if( entry == m_KeyPressBinds.end() ) { return false; }
		if( entry->second.IsEmpty() ) { return false; }

		// Insert into key event queue
		m_KeyEvents.Push( entry->second );
		m_KeyDownList[ inKey ] = true;

		return true;
	}


	bool InputManager::OnKeyRelease( Keys inKey )
	{
		// Check if this key isnt down
		if( !m_KeyDownList[ inKey ] ) { return true; }

		// Get shared lock on key binding list, and lookup the binding
		std::shared_lock< std::shared_mutex > lock( m_BindListMutex );

		auto entry = m_KeyReleaseBinds.find( inKey );
		if( entry == m_KeyReleaseBinds.end() ) { return false; }
		if( entry->second.IsEmpty() ) { return false; }

		// Insert event into key event queue
		m_KeyEvents.Push( entry->second );
		m_KeyDownList[ inKey ] = false;

		return true;
	}


	bool InputManager::OnAxisInput( InputAxis inAxis, float inValue )
	{
		// TODO: Pass raw event into UI system

		// Get shared lock on axis binding list, and lookup the binding
		std::shared_lock< std::shared_mutex > lock( m_BindListMutex );

		auto entry = m_AxisBinds.find( inAxis );
		if( entry == m_AxisBinds.end() ) { return false; }
		if( entry->second.cmd.IsEmpty() ) { return false; }

		// Apply multiplier and invert
		float finalValue = inValue * entry->second.mult * ( entry->second.invert ? -1.f : 1.f );

		m_AxisEvents.Push( std::make_pair( entry->second.cmd, finalValue ) );
		return true;
	}


	void InputManager::DispatchEvents()
	{
		// Get the local player to dispatch events into
		auto game = Engine::GetGame();
		auto localPlayer = game ? game->GetLocalPlayer() : nullptr;

		if( !localPlayer || !localPlayer->IsValid() )
		{
			Console::WriteLine( "[ERROR] InputManager: Failed to dispatch user input events! LocalPlayer was null" );
			return;
		}

		// Pop all input events, and dispatch them
		auto keyEvent = m_KeyEvents.PopValue();
		while( keyEvent.first )
		{
			localPlayer->ProcessKeyBinding( keyEvent.second );
			keyEvent = m_KeyEvents.PopValue();
		}

		auto axisEvent = m_AxisEvents.PopValue();
		while( axisEvent.first )
		{
			localPlayer->ProcessAxisBinding( axisEvent.second.first, axisEvent.second.second );
			axisEvent = m_AxisEvents.PopValue();
		}
	}


	void InputManager::LoadBindings()
	{
		// Clear out any current bindinds
		m_KeyPressBinds.clear();
		m_KeyReleaseBinds.clear();
		m_AxisBinds.clear();

		String filePath( INPUT_BINDINGS_FILE );
		auto f = FileSystem::OpenFile( FilePath( filePath ), FileMode::Read );

		if( !f || !f->IsValid() )
		{
			// There is no file, so lets load the defaults for the current operating system
			// TODO

			// For now, were just going to hard code some defaults
			BindKey( Keys::W, "+forward", true );
			BindKey( Keys::W, "-forward", false );
			BindKey( Keys::S, "+back", true );
			BindKey( Keys::S, "-back", false );
			BindKey( Keys::A, "+left", true );
			BindKey( Keys::A, "-left", false );
			BindKey( Keys::D, "+right", true );
			BindKey( Keys::D, "-right", false );

			BindAxis( InputAxis::MouseX, "view_x", 1.f, false );
			BindAxis( InputAxis::MouseY, "view_y", 1.f, false );
		}
		else
		{
			HYPERION_NOT_IMPLEMENTED( "Reading key bindings from file" );
		}

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




	/*
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
					if( ( It->Type == KeyEvent::Pressed || It->Type == KeyEvent::Any ) && It->Target && It->Target->IsValid() && It->Callback )
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
					if( ( It->Type == KeyEvent::Pressed || It->Type == KeyEvent::Any ) && It->Callback )
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
					if( ( It->Type == KeyEvent::Released || It->Type == KeyEvent::Any ) && It->Target && It->Target->IsValid() && It->Callback )
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
					if( ( It->Type == KeyEvent::Released || It->Type == KeyEvent::Any ) && It->Callback )
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

			for( auto It = rawaxisBindings->second.begin(); It != rawaxisBindings->second.end(); It++ )
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

	*/
}

HYPERION_REGISTER_OBJECT_TYPE( InputManager, Object );