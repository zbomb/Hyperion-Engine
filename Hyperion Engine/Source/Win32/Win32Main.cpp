/*==================================================================================================
	Hyperion Engine
	Source/Win32/Win32Main.cpp
	© 2020, Zachary Berry
==================================================================================================*/

#include "Hyperion/Hyperion.h"
#include "Hyperion/Win32/Win32Headers.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Core/InputManager.h"
#include "../Tests/RenderTests.hpp"
#include "Hyperion/Core/ThreadManager.h"

#include <io.h>
#include <fcntl.h>
#include <corecrt_wstring.h>
#include <thread>

/*
*	Preprocessor Definitions
*/
#define MAX_STRLEN 100

/*
*	Variables
*/
HINSTANCE hInst;
WCHAR szTitle[ MAX_STRLEN ];
WCHAR szWindowClass[ MAX_STRLEN ];
HWND hWindow;
POINT pCursorLastPos;
BOOL bIsCaptured = FALSE;
BOOL bRecapture = FALSE;
POINT pCursorCenter;
int iMouseLastX;
int iMouseLastY;
bool bUsingOSConsole;
bool bFatalError = false;
Hyperion::String sErrorMessage;
std::thread::id tiWinThreadId;



/*
*	Forward Declarations
*/
int Impl_Main( HINSTANCE inInstance, LPWSTR inCmdLine, int inCmdShow );
void Impl_Shutdown();
ATOM RegisterWindow( HINSTANCE inInstance );
BOOL InitInstance( HINSTANCE inInstance, int inCmdShow, UINT32 inWidth, UINT32 inHeight );
LRESULT CALLBACK WndProc( HWND inWindow, UINT inMessage, WPARAM inWParam, LPARAM inLParam );
void BindSystemIOHandles( bool bBindIn, bool bBindOut, bool bBindErr );
void OnMouseCapture( bool bIsCaptured );
void CenterCursor();
Hyperion::Keys TranslateKeyboardScanCode( USHORT inCode, bool E0 );
Hyperion::String GetKeyCodeName( Hyperion::Keys );


int APIENTRY wWinMain( _In_ HINSTANCE inInstance,
					   _In_opt_ HINSTANCE inPrevInstance,
					   _In_ LPWSTR    inCmdLine,
					   _In_ int       inCmdShow )
{
	UNREFERENCED_PARAMETER( inInstance );
	int errValue = 0;
	bUsingOSConsole = false;

	//__try
	//{
		try
		{
			errValue = Impl_Main( inInstance, inCmdLine, inCmdShow );
		}
		catch( ... )
		{
			MessageBox( NULL, L"There was an unhandled exception (native), so the engine had to close.", L"Hyperion Engine Crash", MB_OK );
		}
	//}
	//__except( EXCEPTION_EXECUTE_HANDLER )
	//{
	//	MessageBox( NULL, L"There was an unhandled exception (win32), so the engine had to close.", L"Hyperion Engine Crash", MB_OK );
	//}

	// Shutdown engine systems
	//__try
	//{
		try
		{
			Impl_Shutdown();
		}
		catch( ... )
		{
			MessageBox( NULL, L"There was an unhandled exception (native), during engine shutdown", L"Hyperion Engine Crash", MB_OK );
		}
	//}
	//__except( EXCEPTION_EXECUTE_HANDLER )
	//{
	//	MessageBox( NULL, L"There was an unhandled exception (win32), during engine shutdown", L"Hyperion Engine Crash", MB_OK );
	//}

	return errValue;
}


int Impl_Main( HINSTANCE inInstance, LPWSTR inCmdLine, int inCmdShow )
{
	// Store current thread id
	tiWinThreadId = std::this_thread::get_id();

	// Read command line arguments 
	std::vector< Hyperion::String > cmdLineArgs;

	int argCount = 0;
	LPWSTR* szArgList = CommandLineToArgvW( inCmdLine, &argCount );
	if( szArgList != NULL )
	{
		for( int i = 0; i < argCount; i++ )
		{
			cmdLineArgs.emplace_back( Hyperion::String( szArgList[ i ], Hyperion::StringEncoding::UTF16 ).ToLower() );
		}
	}

	// Check for various flags
	bool bOSConsole = false;
	bool bForceFullscreen = false;
	bool bForceWindowed = false;

	for( auto& arg : cmdLineArgs )
	{
		if( arg.Equals( "-os_console" ) )
		{
			bOSConsole = true;
			bUsingOSConsole = true;
		}
		else if( arg.Equals( "-fullscreen" ) )
		{
			bForceFullscreen = true;
		}
		else if( arg.Equals( "-windowed" ) )
		{
			bForceWindowed = true;
		}
	}

	// Check for OS console mode
	if( bOSConsole )
	{
		AllocConsole();
		BindSystemIOHandles( false, true, true );
	}

	// Ensure fullscreen/windowed flags dont clash
	if( bForceFullscreen && bForceWindowed )
	{
		// Show an error message, and default to fullscreen, but dont close the engine
		MessageBox( hWindow, L"Failed to start engine.. conflicting launch arguments.. -fullscreen and -windowed are both selected!\nThe engine will start in fullscreen mode"
					, L"Hyperion Warning!", MB_OK );
		bForceWindowed = false;
	}

	// Set fatal error callback
	Hyperion::Engine::SetFatalErrorCallback(
		[&] ( const Hyperion::String& inStr )
		{
			// Check if were on the OS layers thread
			if( std::this_thread::get_id() == tiWinThreadId )
			{
				auto finalStr = Hyperion::String::GetSTLWString( Hyperion::String( "[FATAL ERROR] " ).Append( inStr ) );
				MessageBox( hWindow, finalStr.c_str(), L"Hyperion Engine Fatal Error", MB_OK );
				std::terminate();
			}
			else
			{
				// If were not, then we have to 'inject' the fatal error into the thread
				bFatalError = true;
				sErrorMessage = inStr;
			}
		}
	);

	// Get engine instance
	auto eng = Hyperion::Engine::Get();
	if( !eng.IsValid() )
	{
		MessageBox( hWindow, L"Failed to start engine. Couldnt get engine instance!", L"Hyperion Error!", MB_OK );
		return -1;

	}
	
	// Start services such as RTTI, Console, FileSystem...
	if( !eng->InitializeServices( bOSConsole ? Hyperion::FLAG_CONSOLE_OS_OUTPUT : Hyperion::FLAG_NONE ) )
	{
		MessageBox( hWindow, L"Failed to start engine. Services failed to initialize!", L"Hyperion Error!", MB_OK );
		return -1;
	}

	auto res = eng->GetStartupScreenResolution();
	if( bForceFullscreen ) { res.FullScreen = true; }
	else if( bForceWindowed ) { res.FullScreen = false; }

	// Get strings setup for app init
	auto wstrAppTitle = Hyperion::String::GetSTLWString( HYPERION_APP_NAME );
	auto chCount = wstrAppTitle.size() > MAX_STRLEN ? MAX_STRLEN : wstrAppTitle.size();
	wcscpy_s( szTitle, MAX_STRLEN, wstrAppTitle.c_str() );

	//LoadStringW( inInstance, IDC_HYPERION, szWindowClass, MAX_STRLEN ); // Not working...
	std::wstring wstrWindowClass = L"hyperion";
	wcscpy_s( szWindowClass, MAX_STRLEN, wstrWindowClass.c_str() );
	
	// Initialize win32 stuff...
	RegisterWindow( inInstance );
	if( !InitInstance( inInstance, inCmdShow, res.Width, res.Height ) || !hWindow )
	{
		MessageBox( hWindow, L"Failed to start engine. Couldnt initialize Win32 instance!", L"Hyperion Error!", MB_OK );
		return -1;
	}

	HACCEL hAccelerators = LoadAccelerators( inInstance, MAKEINTRESOURCE( IDC_HYPERION ) );

	// Setup RawInput
	RAWINPUTDEVICE rawInputDevices[ 2 ]{};

	rawInputDevices[ 0 ].usUsagePage	= 0x01; 
	rawInputDevices[ 0 ].usUsage		= 0x02;		// Mouse Input
	rawInputDevices[ 0 ].dwFlags		= NULL;
	rawInputDevices[ 0 ].hwndTarget		= hWindow;

	rawInputDevices[ 1 ].usUsagePage	= 0x01;
	rawInputDevices[ 1 ].usUsage		= 0x06;		// Keyboard input
	rawInputDevices[ 1 ].dwFlags		= NULL;
	rawInputDevices[ 1 ].hwndTarget		= hWindow;

	// Register our raw input devices
	if( RegisterRawInputDevices( rawInputDevices, 2, sizeof( RAWINPUTDEVICE ) ) == FALSE )
	{
		MessageBox( hWindow, L"Failed to start engine. Failed to register raw input devices!", L"Hyperion Error!", MB_OK );
		return -1;
	}

	iMouseLastX = -1;
	iMouseLastY = -1;

	// Startup renderer
	if( !eng->InitializeRenderer( hWindow, res, Hyperion::FLAG_NONE ) )
	{
		MessageBox( hWindow, L"Failed to start engine. Renderer failed to initialize!", L"Hyperion Error!", MB_OK );
		return -1;
	}

	// Startup game framework
	if( !eng->InitializeGame( Hyperion::FLAG_NONE ) )
	{
		MessageBox( hWindow, L"Failed to start engine. Game framework failed to initialize!", L"Hyperion Error!", MB_OK );
		return -1;
	}

	// Setup input manager
	auto inputManager = eng->GetInputManagerPtr();
	if( !inputManager.IsValid() )
	{
		MessageBox( hWindow, L"Failed to start engine. Input manager failed!", L"Hyperion Error!", MB_OK );
		return -1;
	}

	inputManager->SetCaptureCallback( &OnMouseCapture );
	inputManager->CaptureMouse();

	// Wait for both renderer and game to initialize
	eng->WaitForInitComplete();
	

	// DEBUG
	// Inject test function into game thread
	auto task = Hyperion::ThreadManager::CreateTask< void >( std::bind( &Hyperion::Tests::RunRendererTests ), Hyperion::THREAD_GAME );

	// New Message Loop
	bool bQuit		= false;
	int exitCode	= 0;

	MSG message{};
	while( !bQuit )
	{
		while( PeekMessage( &message, NULL, 0, 0, PM_REMOVE ) > 0 )
		{
			if( bFatalError ) { break; }

			switch( message.message )
			{
				case WM_QUIT:
				{
					bQuit = true;
					exitCode = message.wParam;
					break;
				}

				case WM_INPUT:
				{
					// Process Raw Input
					UINT dataSize{};
					if( GetRawInputData( (HRAWINPUT) message.lParam, RID_INPUT, NULL, &dataSize, sizeof( RAWINPUTHEADER ) ) != 0 )
					{
						Hyperion::Console::WriteLine( "[ERROR] Win32: Failed to get size of RawInput message! [", GetLastError(), "]" );
						break;
					}

					// Allocate buffer to copy data into
					std::vector< Hyperion::byte > dataBuffer;
					dataBuffer.resize( dataSize );

					if( GetRawInputData( (HRAWINPUT) message.lParam, RID_INPUT, dataBuffer.data(), &dataSize, sizeof( RAWINPUTHEADER ) ) != dataSize )
					{
						Hyperion::Console::WriteLine( "[ERROR] Win32: Failed to get data from RawInput! [", GetLastError(), "]" );
						break;
					}

					RAWINPUT* rawInput = reinterpret_cast<RAWINPUT*>( dataBuffer.data() );

					if( rawInput->header.dwType == RIM_TYPEMOUSE )
					{
						auto& mouseInput	= rawInput->data.mouse;

						// Only accept mouse axis input when mouse is capture
						if( inputManager->IsMouseCaptured() )
						{
							// Translate RawInput mouse message, into something that can be used by our input system
							int deltaX			= 0;
							int deltaY			= 0;

							if( ( mouseInput.usFlags & MOUSE_MOVE_ABSOLUTE ) == MOUSE_MOVE_ABSOLUTE )
							{
								// Mouse info is absolute
								// Set last mouse x and y for the first time
								if( iMouseLastX >= 0 && iMouseLastY >= 0 )
								{
									deltaX = (int) mouseInput.lLastX - iMouseLastX;
									deltaY = (int) mouseInput.lLastY - iMouseLastY;
								}

								iMouseLastX = mouseInput.lLastX;
								iMouseLastY = mouseInput.lLastY;
							}
							else if( mouseInput.lLastX != 0 || mouseInput.lLastY != 0 )
							{
								// Mouse info is relative
								deltaX = (int) mouseInput.lLastX;
								deltaY = (int) mouseInput.lLastY;

							}

							bool bVirtual		= ( mouseInput.usFlags & MOUSE_VIRTUAL_DESKTOP ) == MOUSE_VIRTUAL_DESKTOP;
							float sensitivity	= 1.f; // TODO: Read console var for mouse sensitivity
							int screenWidth		= GetSystemMetrics( bVirtual ? SM_CXVIRTUALSCREEN : SM_CXSCREEN );
							float axisInputX	= (float) deltaX / (float) screenWidth;
							int screenHeight	= GetSystemMetrics( bVirtual ? SM_CYVIRTUALSCREEN : SM_CYSCREEN );
							float axisInputY	= (float) deltaY / (float) screenHeight;

							static const float mouseSensitivity = 1.7f;

							inputManager->PushAxisState( Hyperion::InputAxis::MouseX, axisInputX * 100.f * mouseSensitivity , true );
							inputManager->PushAxisState( Hyperion::InputAxis::MouseY, axisInputY * 100.f * mouseSensitivity, true );

							// Now, lets check for mouse actions, starting with the mouse wheel
							if( ( mouseInput.usButtonFlags & RI_MOUSE_WHEEL ) == RI_MOUSE_WHEEL ||
								( mouseInput.usButtonFlags & RI_MOUSE_HWHEEL ) == RI_MOUSE_HWHEEL )
							{
								short value = (short) mouseInput.usButtonData;
								float deltaTicks = (float) value / (float) WHEEL_DELTA;
								bool bHorizontal = ( mouseInput.usButtonFlags & RI_MOUSE_HWHEEL ) == RI_MOUSE_HWHEEL;

								if( bHorizontal )
								{
									// TODO: Push axis event
								}
								else
								{
									// TODO: Push axis event
								}
							}
						}

						// Finally, check if any mouse buttons were pressed or released
						// Left Click
						if( ( mouseInput.usButtonData & RI_MOUSE_LEFT_BUTTON_DOWN ) == RI_MOUSE_LEFT_BUTTON_DOWN )
						{
							inputManager->PushKeyState( Hyperion::Keys::MOUSE1, true );
						}
						else if( ( mouseInput.usButtonData & RI_MOUSE_LEFT_BUTTON_UP ) == RI_MOUSE_LEFT_BUTTON_UP )
						{
							inputManager->PushKeyState( Hyperion::Keys::MOUSE1, false );
						}

						// RIght click
						if( ( mouseInput.usButtonData & RI_MOUSE_RIGHT_BUTTON_DOWN ) == RI_MOUSE_RIGHT_BUTTON_DOWN )
						{
							inputManager->PushKeyState( Hyperion::Keys::MOUSE2, true );
						}
						else if( ( mouseInput.usButtonData & RI_MOUSE_RIGHT_BUTTON_UP ) == RI_MOUSE_RIGHT_BUTTON_UP )
						{
							inputManager->PushKeyState( Hyperion::Keys::MOUSE2, false );
						}

						// Middle Click
						if( ( mouseInput.usButtonData & RI_MOUSE_MIDDLE_BUTTON_DOWN ) == RI_MOUSE_MIDDLE_BUTTON_DOWN )
						{
							inputManager->PushKeyState( Hyperion::Keys::MOUSE3, true );
						}
						else if( ( mouseInput.usButtonData & RI_MOUSE_MIDDLE_BUTTON_UP ) == RI_MOUSE_MIDDLE_BUTTON_UP )
						{
							inputManager->PushKeyState( Hyperion::Keys::MOUSE3, false );
						}

						// Mouse Button 4
						if( ( mouseInput.usButtonData & RI_MOUSE_BUTTON_4_DOWN ) == RI_MOUSE_BUTTON_4_DOWN )
						{
							inputManager->PushKeyState( Hyperion::Keys::MOUSE4, true );
						}
						else if( ( mouseInput.usButtonData & RI_MOUSE_BUTTON_4_UP ) == RI_MOUSE_BUTTON_4_UP )
						{
							inputManager->PushKeyState( Hyperion::Keys::MOUSE4, false );
						}

						// Mouse Button 5
						if( ( mouseInput.usButtonData & RI_MOUSE_BUTTON_5_DOWN ) == RI_MOUSE_BUTTON_5_DOWN )
						{
							inputManager->PushKeyState( Hyperion::Keys::MOUSE5, true );
						}
						else if( ( mouseInput.usButtonData & RI_MOUSE_BUTTON_5_UP ) == RI_MOUSE_BUTTON_5_UP )
						{
							inputManager->PushKeyState( Hyperion::Keys::MOUSE5, false );
						}
					}
					else if( rawInput->header.dwType == RIM_TYPEKEYBOARD )
					{
						// Determine which keys were pressed and which keys were released this frame
						auto& keyInput = rawInput->data.keyboard;

						// First, lets read the flags
						bool bWasPress = true;

						if( ( keyInput.Flags & RI_KEY_BREAK ) == RI_KEY_BREAK )
						{
							bWasPress = false;
						}

						bool bE0 = ( ( keyInput.Flags & RI_KEY_E0 ) == RI_KEY_E0 );

						// Next, we need to determine which key is in the message
						auto key = TranslateKeyboardScanCode( keyInput.MakeCode, bE0 );
						if( key == Hyperion::Keys::NONE )
						{
							// DEBUG
							//Hyperion::Console::WriteLine( "[DEBUG] Win32: Unknown key was pressed.. Make Code: ", keyInput.MakeCode, " E0: ", bE0 ? "true" : "false" );
						}
						else
						{
							//Hyperion::Console::WriteLine( "[DEBUG] Win32: Key pressed was \"", GetKeyCodeName( key ), "\"  [", bWasPress ? "PRESS" : "RELEASE", "]" );
							inputManager->PushKeyState( key, bWasPress );
						}
					}

					break;
				}
			} // Message switch end

			TranslateMessage( &message );
			DispatchMessage( &message );

			if( bQuit ) { break; }
		}

		if( bFatalError ) { break; }

	} // Message loop end

	if( inputManager ) { inputManager->ReleaseMouse(); }

	// Check if we exited due to a fatal error, or naturally
	if( bFatalError )
	{
		auto finalStr = Hyperion::String::GetSTLWString( Hyperion::String( "[FATAL ERROR] " ).Append( sErrorMessage ) );
		MessageBox( hWindow, finalStr.c_str(), L"Hyperion Engine Fatal Error", MB_OK );
		std::terminate();
	}
	
	return exitCode;
}


void Impl_Shutdown()
{
	Hyperion::Console::WriteLine( "[Win32] Application closing..." );

	auto eng = Hyperion::Engine::Get();
	if( !eng )
	{
		MessageBox( NULL, L"Failed to properly shutdown engine, couldnt get instance during shutdown function", L"Hyperion Shutdown Error", MB_OK );
		return;
	}

	eng->Stop();

	if( bUsingOSConsole )
	{
		FreeConsole();
	}
}


ATOM RegisterWindow( HINSTANCE inInstance )
{
	WNDCLASSEXW windowClass;

	windowClass.cbSize = sizeof( WNDCLASSEX );

	windowClass.style          = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc    = WndProc;
	windowClass.cbClsExtra     = 0;
	windowClass.cbWndExtra     = 0;
	windowClass.hInstance      = inInstance;
	windowClass.hIcon          = LoadIcon( inInstance, MAKEINTRESOURCE( IDI_ICON1 ) );
	windowClass.hCursor        = LoadCursor( nullptr, IDC_ARROW );
	windowClass.hbrBackground  = (HBRUSH) ( COLOR_WINDOW + 1 );

	// Convert string literal to type used by windows
	std::string narrowName = HYPERION_APP_NAME;
	wchar_t wideName[ 40 ];
	size_t numChars = 0;
	mbstowcs_s( &numChars, wideName, narrowName.c_str(), 40 );

	windowClass.lpszMenuName   = wideName;
	windowClass.lpszClassName  = szWindowClass;
	windowClass.hIconSm        = LoadIcon( windowClass.hInstance, MAKEINTRESOURCE( IDI_ICON2 ) );

	return RegisterClassExW( &windowClass );
}


BOOL InitInstance( HINSTANCE inInstance, int inCmdShow, UINT32 inWidth, UINT32 inHeight )
{
	// Create the window through win32 api
	HWND hWnd = CreateWindowW( szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
							   CW_USEDEFAULT, 0, inWidth, inHeight, nullptr, nullptr, inInstance, nullptr );

	if( !hWnd ) { return FALSE; }

	// Store handles in globals
	hInst		= inInstance;
	hWindow		= hWnd;

	// Show the window to the user
	ShowWindow( hWnd, inCmdShow );
	UpdateWindow( hWnd );

	return TRUE;
}


LRESULT CALLBACK WndProc( HWND inWindow, UINT inMessage, WPARAM inWParam, LPARAM inLParam )
{
	switch( inMessage )
	{
		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC h = BeginPaint( inWindow, &paint );
			// TODO: Any HDC drawing code here?
			EndPaint( inWindow, &paint );
			break;
		}

		case WM_ACTIVATE:
		{
			// If the game is gaining focus, we want to capture the mouse, if were loosing focus, we want to release the mouse
			if( LOWORD( inWParam ) == WA_INACTIVE )
			{
				// Loosing focus
				Hyperion::Engine::GetInputManager()->ReleaseMouse();
			}
			else if( LOWORD( inWParam ) == WA_ACTIVE )
			{
				// Grabbing focus
				Hyperion::Engine::GetInputManager()->CaptureMouse();
			}
			else if( LOWORD( inWParam ) == WA_CLICKACTIVE )
			{
				Hyperion::Engine::GetInputManager()->CaptureMouse();
			}

			break;
		}

	case WM_DESTROY:

		Hyperion::Console::WriteLine( "---------------------> WM_DESTROY" );

		PostQuitMessage( inWParam );
		break;

	default:
		
		return DefWindowProc( inWindow, inMessage, inWParam, inLParam );
	}

	return 0;
}


void BindSystemIOHandles( bool bBindIn, bool bBindOut, bool bBindErr )
{
	if( bBindIn )
	{
		HYPERION_NOT_IMPLEMENTED( "Binding external console input is not implemented yet" );
	}

    if( bBindOut )
    {
        FILE* dummyFile;
        freopen_s( &dummyFile, "nul", "w", stdout );
    }

    if( bBindErr )
    {
        FILE* dummyFile;
        freopen_s( &dummyFile, "nul", "w", stderr );
    }

    if( bBindOut )
    {
        HANDLE stdHandle = GetStdHandle( STD_OUTPUT_HANDLE );
        if( stdHandle != INVALID_HANDLE_VALUE )
        {
            int fileDescriptor = _open_osfhandle( (intptr_t) stdHandle, _O_U16TEXT );
            if( fileDescriptor != -1 )
            {
                FILE* file = _fdopen( fileDescriptor, "w" );
                if( file != NULL )
                {
                    int dup2Result = _dup2( _fileno( file ), _fileno( stdout ) );
                    if( dup2Result == 0 )
                    {
                        setvbuf( stdout, NULL, _IONBF, 0 );
                    }
                }
            }
        }
    }

    if( bBindErr )
    {
        HANDLE stdHandle = GetStdHandle( STD_ERROR_HANDLE );
        if( stdHandle != INVALID_HANDLE_VALUE )
        {
            int fileDescriptor = _open_osfhandle( (intptr_t) stdHandle, _O_U16TEXT );
            if( fileDescriptor != -1 )
            {
                FILE* file = _fdopen( fileDescriptor, "w" );
                if( file != NULL )
                {
                    int dup2Result = _dup2( _fileno( file ), _fileno( stderr ) );
                    if( dup2Result == 0 )
                    {
                        setvbuf( stderr, NULL, _IONBF, 0 );
                    }
                }
            }
        }
    }

    if( bBindOut )
    {
        std::wcout.clear();
        std::cout.clear();
    }

    if( bBindErr )
    {
        std::wcerr.clear();
        std::cerr.clear();
    }
}


void OnMouseCapture( bool bShouldCapture )
{
	if( bShouldCapture )
	{
		// If were already captured, then do nothing
		// Also, if we dont have a window handle, then do nothing
		if( bIsCaptured || !hWindow )
		{
			return;
		}

		// Store the position of the cursor, so when we uncapture it, we can set it back to here
		GetCursorPos( &pCursorLastPos );

		// Capture the mouse by hiding the system version, position it to the last known location
		// and bounding it to the window bounds
		RECT WindowPosition;

		//CenterCursor();
		GetWindowRect( hWindow, &WindowPosition );

		ClipCursor( &WindowPosition );
		bIsCaptured = true;

		// Just ensuring the mouse is hidden..
		while( ShowCursor( FALSE ) > 0 );
	}
	else
	{
		if( !bIsCaptured )
		{
			return;
		}

		// Uncapture the mouse by restoring it to the last known position, unbound and show
		if( !SetCursorPos( pCursorLastPos.x, pCursorLastPos.y ) )
		{
			// Error
			Hyperion::Console::WriteLine( "[ERROR] Win32: Failed to restore cursor position!" );
		}

		ClipCursor( NULL );
		bIsCaptured = false;

		ShowCursor( TRUE );

		// Ensure the window isnt recaptured automatically
		bRecapture = FALSE;
	}

	// Update cached cursor position used for calculating mouse input
	POINT screenSpacePoint;
	if( GetCursorPos( &screenSpacePoint ) )
	{
		if( ScreenToClient( hWindow, &screenSpacePoint ) )
		{
			pCursorCenter = screenSpacePoint;
		}
	}
}


void CenterCursor()
{
	if( hWindow == NULL )
		return;

	// Get window bounds and map it to window space
	RECT WindowPosition;
	GetWindowRect( hWindow, &WindowPosition );

	MapWindowPoints( HWND_DESKTOP, hWindow, (LPPOINT) &WindowPosition, 2 );

	// Calculate the center of the window
	LONG W = WindowPosition.right - WindowPosition.left;
	LONG H = WindowPosition.bottom - WindowPosition.top;

	LONG CenterX = WindowPosition.left + ( W / 2 );
	LONG CenterY = WindowPosition.top + ( H / 2 );

	// Update cursor position
	SetCursorPos( CenterX, CenterY );

	POINT centerPoint;
	centerPoint.x = CenterX;
	centerPoint.y = CenterY;

	if( ScreenToClient( hWindow, &centerPoint ) )
	{
		pCursorCenter = centerPoint;
	}
}


Hyperion::String GetKeyCodeName( Hyperion::Keys inKey )
{
	using namespace Hyperion;

	switch( inKey )
	{
	case Keys::A: { return "A"; }
	case Keys::B: { return "B"; }
	case Keys::C: { return "C"; }
	case Keys::D: { return "D"; }
	case Keys::E: { return "E"; }
	case Keys::F: { return "F"; }
	case Keys::G: { return "G"; }
	case Keys::H: { return "H"; }
	case Keys::I: { return "I"; }
	case Keys::J: { return "J"; }
	case Keys::K: { return "K"; }
	case Keys::L: { return "L"; }
	case Keys::M: { return "M"; }
	case Keys::N: { return "N"; }
	case Keys::O: { return "O"; }
	case Keys::P: { return "P"; }
	case Keys::Q: { return "Q"; }
	case Keys::R: { return "R"; }
	case Keys::S: { return "S"; }
	case Keys::T: { return "T"; }
	case Keys::U: { return "U"; }
	case Keys::V: { return "V"; }
	case Keys::W: { return "W"; }
	case Keys::X: { return "X"; }
	case Keys::Y: { return "Y"; }
	case Keys::Z: { return "Z"; }
	case Keys::ZERO: { return "0"; }
	case Keys::ONE: { return "1"; }
	case Keys::TWO: { return "2"; }
	case Keys::THREE: { return "3"; }
	case Keys::FOUR: { return "4"; }
	case Keys::FIVE: { return "5"; }
	case Keys::SIX: { return "6"; }
	case Keys::SEVEN: { return "7"; }
	case Keys::EIGHT: { return "8"; }
	case Keys::NINE: { return "9"; }
	case Keys::TILDE: {return "`"; }
	case Keys::MINUS: { return "-"; }
	case Keys::EQUALS: { return "="; }
	case Keys::FORWARDSLASH: { return "/"; }
	case Keys::BACKSPACE: { return "BCKSP"; }
	case Keys::SPACE: { return "SPACE"; }
	case Keys::TAB: { return "TAB"; }
	case Keys::CAPS: { return "CAPS"; }
	case Keys::PRINTSCREEN: { return "PRSCR"; }
	case Keys::LSHIFT: { return "LSHIFT"; }
	case Keys::RCTRL: { return "RCTRL"; }
	case Keys::LCTRL: { return "LCTRL"; }
	case Keys::RALT: { return "RALT"; }
	case Keys::LALT: { return "LALT"; }
	case Keys::RSHIFT: { return "RSHIFT"; }
	case Keys::ESCAPE: { return "ESC"; }
	case Keys::F1: { return "F1"; }
	case Keys::F2: { return "F2"; }
	case Keys::F3: { return "F3"; }
	case Keys::F4: { return "F4"; }
	case Keys::F5: { return "F5"; }
	case Keys::F6: { return "F6"; }
	case Keys::F7: { return "F7"; }
	case Keys::F8: { return "F8"; }
	case Keys::F9: { return "F9"; }
	case Keys::F10: { return "F10"; }
	case Keys::F11: { return "F11"; }
	case Keys::F12: { return "F12"; }
	case Keys::SCROLL_LOCK: { return "SCRLK"; }
	case Keys::LBRACKET: { return "["; }
					    
	// Keypad
	case Keys::NUM_DIV: { return "NUM/"; }
	case Keys::BACKSLASH: { return "\\"; }
	case Keys::NUM_MULT: { return "NUM*"; }
	case Keys::NUM_SUB: { return "NUM-"; }
	case Keys::NUM_ADD: { return "NUM+"; }
	case Keys::NUM_ENTER: { return "NUMENTER"; }
	case Keys::ENTER: { return "ENTER"; }
	case Keys::DEL: { return "DEL"; }
	case Keys::NUM_DECIMAL: { return "NUM."; }
	case Keys::INSERT: {  return "INS"; }
	case Keys::NUM0: { return "NUM0"; }
	case Keys::END: { return "END"; }
	case Keys::NUM1: { return "NUM1"; }
	case Keys::DOWN: { return "DOWN"; }
	case Keys::NUM2: { return "NUM2"; }
	case Keys::PAGEDOWN: { return "PGDWN"; }
	case Keys::NUM3: { return "NUM3"; }
	case Keys::LEFT: { return "LEFT"; }
	case Keys::NUM4: { return "NUM4"; }
	case Keys::NUM5: { return "NUM5"; }
	case Keys::RIGHT: { return "RIGHT"; }
	case Keys::NUM6: { return "NUM6"; }
	case Keys::HOME: { return "HOME"; }
	case Keys::NUM7: { return "NUM7"; }
	case Keys::UP: { return "UP"; }
	case Keys::NUM8: { return "NUM8"; }
	case Keys::PAGEUP: { return "PGUP"; }
	case Keys::NUM9: { return "NUM9"; }
	case Keys::RBRACKET: { return "]"; }
	case Keys::SEMICOLON: { return ";"; }
	case Keys::QUOTE: { return "'"; }
	case Keys::COMMA: { return ","; }
	case Keys::PERIOD: { return "."; }
	default: { return "NONE"; } 
	}
}


Hyperion::Keys TranslateKeyboardScanCode( USHORT inCode, bool E0 )
{
	using namespace Hyperion;

	//inCode = inCode & 0x00'11; // Were only concerned with the last byte of the code
	switch( inCode )
	{
	case 0x1E: { return Keys::A; }
	case 0x30: { return Keys::B; }
	case 0x2E: { return Keys::C; }
	case 0x20: { return Keys::D; }
	case 0x12: { return Keys::E; }
	case 0x21: { return Keys::F; }
	case 0x22: { return Keys::G; }
	case 0x23: { return Keys::H; }
	case 0x17: { return Keys::I; }
	case 0x24: { return Keys::J; }
	case 0x25: { return Keys::K; }
	case 0x26: { return Keys::L; }
	case 0x32: { return Keys::M; }
	case 0x31: { return Keys::N; }
	case 0x18: { return Keys::O; }
	case 0x19: { return Keys::P; }
	case 0x10: { return Keys::Q; }
	case 0x13: { return Keys::R; }
	case 0x1F: { return Keys::S; }
	case 0x14: { return Keys::T; }
	case 0x16: { return Keys::U; }
	case 0x2F: { return Keys::V; }
	case 0x11: { return Keys::W; }
	case 0x2D: { return Keys::X; }
	case 0x15: { return Keys::Y; }
	case 0x2C: { return Keys::Z; }
	case 0x0B: { return Keys::ZERO; }
	case 0x02: { return Keys::ONE; }
	case 0x03: { return Keys::TWO; }
	case 0x04: { return Keys::THREE; }
	case 0x05: { return Keys::FOUR; }
	case 0x06: { return Keys::FIVE; }
	case 0x07: { return Keys::SIX; }
	case 0x08: { return Keys::SEVEN; }
	case 0x09: { return Keys::EIGHT; }
	case 0x0A: { return Keys::NINE; }
	case 0x29: { return Keys::TILDE; }
	case 0x0C: { return Keys::MINUS; }
	case 0x0D: { return Keys::EQUALS; }
	case 0x2B: { return Keys::FORWARDSLASH; }
	case 0x0E: { return Keys::BACKSPACE; }
	case 0x39: { return Keys::SPACE; }
	case 0x0F: { return Keys::TAB; }
	case 0x3A: { return Keys::CAPS; }
	case 0x2A: { return E0 ? Keys::PRINTSCREEN : Keys::LSHIFT; }
	case 0x1D: { return E0 ? Keys::RCTRL : Keys::LCTRL; }
	case 0x38: { return E0 ? Keys::RALT : Keys::LALT; }
	case 0x36: { return Keys::RSHIFT; }
	case 0x01: { return Keys::ESCAPE; }
	case 0x3B: { return Keys::F1; }
	case 0x3C: { return Keys::F2; }
	case 0x3D: { return Keys::F3; }
	case 0x3E: { return Keys::F4; }
	case 0x3F: { return Keys::F5; }
	case 0x40: { return Keys::F6; }
	case 0x41: { return Keys::F7; }
	case 0x42: { return Keys::F8; }
	case 0x43: { return Keys::F9; }
	case 0x44: { return Keys::F10; }
	case 0x57: { return Keys::F11; }
	case 0x58: { return Keys::F12; }
	case 0x46: { return Keys::SCROLL_LOCK; }
	case 0x1A: { return Keys::LBRACKET; }

	// Keypad
	case 0x35: { return E0 ? Keys::NUM_DIV : Keys::BACKSLASH; }
	case 0x37: { return E0 ? Keys::PRINTSCREEN : Keys::NUM_MULT; }
	case 0x4A: { return Keys::NUM_SUB; }
	case 0x4E: { return Keys::NUM_ADD; }
	case 0x1C: { return E0 ? Keys::NUM_ENTER : Keys::ENTER; }
	case 0x53: { return E0 ? Keys::DEL : Keys::NUM_DECIMAL; }
	case 0x52: { return E0 ? Keys::INSERT : Keys::NUM0; }
	case 0x4F: { return E0 ? Keys::END : Keys::NUM1; }
	case 0x50: { return E0 ? Keys::DOWN : Keys::NUM2; }
	case 0x51: { return E0 ? Keys::PAGEDOWN : Keys::NUM3; }
	case 0x4B: { return E0 ? Keys::LEFT : Keys::NUM4; }
	case 0x4C: { return Keys::NUM5; }
	case 0x4D: { return E0 ? Keys::RIGHT : Keys::NUM6; }
	case 0x47: { return E0 ? Keys::HOME : Keys::NUM7; }
	case 0x48: { return E0 ? Keys::UP : Keys::NUM8; }
	case 0x49: { return E0 ? Keys::PAGEUP : Keys::NUM9; }
	case 0x1B: { return Keys::RBRACKET; }
	case 0x27: { return Keys::SEMICOLON; }
	case 0x28: { return Keys::QUOTE; }
	case 0x33: { return Keys::COMMA; }
	case 0x34: { return Keys::PERIOD; }
	default: { return Keys::NONE; }

	}
}