/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/InputManager.h
	� 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Object.h"
#include "Hyperion/Core/Types/ConcurrentQueue.h"
#include "Hyperion/InputTypes.h"

#include <shared_mutex>


namespace Hyperion
{

	constexpr const char* INPUT_BINDINGS_FILE				= "data/key_bindings.cfg";


	class InputManager : public Object
	{
		
	private:

		struct AxisBinding
		{
			uint32 Identifier;
			float Multiplier;
			bool bInvert;
			bool bUseFullRange;
		};


		struct ButtonBinding
		{
			uint32 Identifier;
			bool bIsAction;
			bool bInvert; // If this is an action, this determins if action is called on release instead. If this is a state binding, it inverts the button state
		};

		struct ButtonState
		{
			bool bActionState;
			bool bState;
		};

		struct ButtonUpdate
		{
			bool bIsAction;
			bool bState;
		};

		// Binding lookup happens in OS thread, an event is created and placed into the concurrent queue
		std::map< Hyperion::Keys, ButtonBinding > m_ButtonBindings;
		std::map< Hyperion::InputAxis, AxisBinding > m_AxisBindings;
		std::map< uint32, bool > m_BoundEvents;

		std::shared_mutex m_BindingsMutex;

		// Then, every game tick, the concurrent queue is read, and the binding state is updated
		std::map< uint32, ButtonState > m_ActionStates;
		std::map< uint32, bool > m_ButtonStates;
		std::map< uint32, float > m_AxisStates;

		ConcurrentQueue< std::pair< uint32, ButtonUpdate > > m_ButtonUpdates;
		ConcurrentQueue< std::pair< uint32, float > > m_AxisUpdates;
		
		bool m_bIsMouseCaptured;
		std::function< void( bool ) > m_CaptureCallback;

		/*
		*	There are 3 types of inputs
		*	1. Scalar Input
		*	2. State Input
		*	3. Action Input
		* 
		*	Scalars have an associated float value, either between [0,1] or [-1,1]
		*	Whenever a scalar input happens, its stored for a frame in the axis state table, for other objects to poll
		* 
		*	State inputs can be polled at any time, and it checks if a button is currently pressed down
		* 
		*	Action inputs check for a button being pressed (or released), and only stays in the cache for a single frame
		*/


	public:

		InputManager();

		bool AddActionBinding( Keys inKey, uint32 inIdentifier, bool bOnRelease = false );
		bool AddStateBinding( Keys inKey, uint32 inIdentifier, bool bInvert = false );
		bool AddScalarBinding( InputAxis inAxis, uint32 inIdentifier, bool bUseFullRange = false, bool bInvertValues = false, float inMultiplier = 1.f );

		bool RemoveButtonBind( Keys inKey );
		bool RemoveAxisBind( InputAxis inAxis );
		bool RemoveBind( uint32 inIdentifier );
		void RemoveAllBindings();

		void LoadBindingsFromDisk();
		void ProcessUpdates();

		void PushKeyState( Keys inKey, bool inState );
		void PushAxisState( InputAxis inAxis, float inState, bool bFullRange = false );

		bool PollAction( uint32 inIdentifier );
		bool PollState( uint32 inIdentifier );
		float PollScalar( uint32 inIdentifier );

		inline bool IsMouseCaptured() const { return m_bIsMouseCaptured; }
		void CaptureMouse();
		void ReleaseMouse();
		void SetCaptureCallback( std::function< void( bool ) > Callback );


		/*
		*	OLD SYSTEM
		

	private:

		struct AxisBinding
		{
			String cmd;
			float mult;
			bool invert;
		};

		std::map< Keys, bool > m_KeyDownList;
		std::map< Keys, String > m_KeyPressBinds;
		std::map< Keys, String > m_KeyReleaseBinds;
		std::map< InputAxis, AxisBinding > m_AxisBinds;

		std::shared_mutex m_BindListMutex;

		bool m_bIsMouseCaptured;
		std::function< void( bool ) > m_CaptureCallback;

		ConcurrentQueue< String > m_KeyEvents;
		ConcurrentQueue< std::pair< String, float > > m_AxisEvents;

	public:

		InputManager();

		bool BindKey( Keys inKey, const String& inCmd, bool bPress );
		bool BindAxis( InputAxis inAxis, const String& inCmd, float inMult, bool bInvert );

		bool UnbindKey( const String& inCmd );
		bool UnbindAxis( const String& inCmd );
		bool UnbindKey( Keys inKey );
		bool UnbindAxis( InputAxis inAxis );

		void ClearBindings();

		bool OnKeyPress( Keys inKey );
		bool OnKeyRelease( Keys inKey );
		bool OnAxisInput( InputAxis inAxis, float inValue );

		void DispatchEvents();

		void LoadBindings();

		inline bool IsMouseCaptured() const { return m_bIsMouseCaptured; }
		void CaptureMouse();
		void ReleaseMouse();
		void SetCaptureCallback( std::function< void( bool ) > Callback );
		*/
	};


	/////////////////////////////////////////// Old Code //////////////////////////////////////////////

	/*
		Forward Declarations
	
	class Entity;

	/*
		KeyPressEvent
		* Structure holding info about a key event, passed as a parameter into key press callbacks
	
	struct KeyPressEvent
	{
		Keys Key;
		KeyEvent Type;
	};

	/*
		InputAxisEvent
		* Structure holding info about an axis event, passed as a parameter into axis callbacks
	
	struct InputAxisEvent
	{
		InputAxis Axis;
		float Value;
	};
	
	/*
		KeyBindInfo
		* Structure to pass as a parameter to InputManager::BindKey
	
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


	class InputManager : public Object
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
		
		std::map< Keys, std::vector< KeyMapping > > m_KeyMappings;
		std::map< InputAxis, std::vector< AxisMapping > > m_AxisMappings;

		std::map< Keys, std::vector< RawKeyMapping > > m_RawKeyMappings;
		std::map< InputAxis, std::vector< RawAxisMapping > > m_RawAxisMappings;

		/*
			Data Members
		
		bool m_bShutdown;
		bool m_KeyStates[ 256 ];
		bool m_bIsMouseCaptured;
		std::function< void( bool ) > m_CaptureCallback;
		Point< int > m_MousePosition;

		/*
			Thread-Safe Event Queue
		
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
	*/

}