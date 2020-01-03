/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/InputManager.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Object.h"

#include <vector>
#include <functional>
#include <map>
#include <chrono>
#include <queue>
#include <mutex>


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class Entity;

	/*
		KeyPressEvent
		* Structure holding info about a key event, passed as a parameter into key press callbacks
	*/
	struct KeyPressEvent
	{
		Keys Key;
		KeyEvent Type;
	};

	/*
		InputAxisEvent
		* Structure holding info about an axis event, passed as a parameter into axis callbacks
	*/
	struct InputAxisEvent
	{
		InputAxis Axis;
		float Value;
	};
	
	/*
		KeyBindInfo
		* Structure to pass as a parameter to InputManager::BindKey
	*/
	struct KeyBindInfo
	{
		Keys Key;
		KeyEvent Type;
		bool bRequireShift;
		bool bRequireAlt;
		bool bRequireCtrl;

		KeyBindInfo()
		{
			Key				= Keys::NONE;
			Type			= KeyEvent::None;
			bRequireShift	= false;
			bRequireAlt		= false;
			bRequireCtrl	= false;
		}
	};

	/*
		AxisBindInfo
		* Structure to pass as a parameter to InputManager::BindAxis
	*/
	struct AxisBindInfo
	{
		InputAxis Axis;
		float Mult;
		bool bRequireShift;
		bool bRequireAlt;
		bool bRequireCtrl;

		AxisBindInfo()
		{
			Axis			= InputAxis::None;
			Mult			= 1.f;
			bRequireShift	= false;
			bRequireAlt		= false;
			bRequireCtrl	= false;
		}
	};


	class InputManager
	{

	private:

		// Private structure to hold info about a key mapping
		struct KeyMapping
		{
			Keys Key;
			
			bool bRequireShift;
			bool bRequireAlt;
			bool bRequireCtrl;

			KeyEvent Type;

			std::function< bool( const KeyPressEvent& ) > Callback;

			HypPtr< Entity > Target;
		};

		struct RawKeyMapping
		{
			Keys Key;

			bool bRequireShift;
			bool bRequireAlt;
			bool bRequireCtrl;

			KeyEvent Type;

			std::function< bool( const KeyPressEvent& ) > Callback;

			void* Target;
		};

		// Private structure to hold info about axis mapping
		struct AxisMapping
		{
			InputAxis Axis;
			float Mult;

			std::function< bool( const InputAxisEvent& ) > Callback;

			HypPtr< Entity > Target;
		};

		struct RawAxisMapping
		{
			InputAxis Axis;
			float Mult;

			std::function< bool( const InputAxisEvent& ) > Callback;

			void* Target;
		};

		// Private structure to hold info about a key press event from the OS
		struct RawKeyEvent
		{
			Keys Key;
			KeyEvent Type;
		};

		// Private structure to hold raw axis event from the OS
		struct RawAxisEvent
		{
			InputAxis Axis;
			int Delta;
			int NewValue;
		};

		/*
			Axis and Key Mappings
		*/
		std::map< Keys, std::vector< KeyMapping > > m_KeyMappings;
		std::map< InputAxis, std::vector< AxisMapping > > m_AxisMappings;

		std::map< Keys, std::vector< RawKeyMapping > > m_RawKeyMappings;
		std::map< InputAxis, std::vector< RawAxisMapping > > m_RawAxisMappings;

		/*
			Data Members
		*/
		bool m_bShutdown;
		bool m_KeyStates[ 256 ];
		bool m_bIsMouseCaptured;
		std::function< void( bool ) > m_CaptureCallback;
		Point< int > m_MousePosition;

		/*
			Thread-Safe Event Queue
		*/
		std::queue< RawKeyEvent > m_KeyEventQueue;
		std::queue< RawAxisEvent > m_AxisEventQueue;

		std::mutex m_KeyMutex;
		std::mutex m_AxisMutex;

		void DispatchKeyPress( Keys inKey );
		void DispatchKeyRelease( Keys inKey );
		void DispatchAxisEvent( InputAxis Axis, int Value, double Delta );

	public:

		InputManager();
		~InputManager();

		void ClearAllBindings();
		
		void BindKeyRaw( const KeyBindInfo& inParams, std::function< bool( const KeyPressEvent& ) > Callback, void* Address );
		void BindAxisRaw( const AxisBindInfo& inParams, std::function< bool( const InputAxisEvent& ) > Callback, void* Address );

		void ClearRawBindings( void* classAddress );

		void BindKey( const KeyBindInfo& inParams, std::function< bool( const KeyPressEvent& ) > Callback, const HypPtr< Entity >& Target );
		void BindAxis( const AxisBindInfo& inParams, std::function< bool( const InputAxisEvent& ) > Callback, const HypPtr< Entity >& Target );

		void ClearBindings( const HypPtr< Entity >& Target );

		void HandleKeyPress( Keys inKey );
		void HandleKeyRelease( Keys inKey );
		void HandleAxisInput( InputAxis Type, int Delta, int NewPos );

		void DispatchQueue( double Delta );

		bool IsKeyDown( Keys inKey );

		inline bool IsMouseCaptured() const { return m_bIsMouseCaptured; }
		void CaptureMouse();
		void ReleaseMouse();
		void SetCaptureCallback( std::function< void( bool ) > Callback );

		inline Point< int > GetMousePosition() const { return m_MousePosition; }


	};


}