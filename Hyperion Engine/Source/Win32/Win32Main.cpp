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



/*
*	Forward Declarations
*/
int Impl_Main( HINSTANCE inInstance, LPWSTR inCmdLine, int inCmdShow );
void Impl_Shutdown();
ATOM RegisterWindow( HINSTANCE inInstance );
BOOL InitInstance( HINSTANCE inInstance, int inCmdShow, UINT32 inWidth, UINT32 inHeight );
LRESULT CALLBACK WndProc( HWND inWindow, UINT inMessage, WPARAM inWParam, LPARAM inLParam );
void BindSystemIOHandles( bool bBindIn, bool bBindOut, bool bBindErr );
bool HandleMouseInput( MSG& inMsg, Hyperion::InputManager& inManager );
bool HandleKeyboardInput( MSG& inMsg, Hyperion::InputManager& inManager );
void OnMouseCapture( bool bIsCaptured );
void CenterCursor();
Hyperion::Keys TranslateMouseButton( MSG& inMessage, bool& outWasPress );
Hyperion::Keys TranslateKeyboardButton( MSG& inMessage, bool& outWasPress );



int APIENTRY wWinMain( _In_ HINSTANCE inInstance,
					   _In_opt_ HINSTANCE inPrevInstance,
					   _In_ LPWSTR    inCmdLine,
					   _In_ int       inCmdShow )
{
	UNREFERENCED_PARAMETER( inInstance );
	int errValue = 0;

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

	// Main message loop
	MSG message;
	while( GetMessage( &message, nullptr, 0, 0 ) )
	{
		if( !TranslateAccelerator( message.hwnd, hAccelerators, &message ) )
		{
			// Check for user input
			bool bHandled = false;
			if( message.message >= WM_MOUSEFIRST && message.message <= WM_MOUSELAST )
			{
				bHandled = HandleMouseInput( message, *inputManager );
			}
			else if( message.message >= WM_KEYFIRST && message.message <= WM_KEYLAST )
			{
				// Check if we clicked the windows button, so we can release the cursor
				if( message.wParam == VK_LWIN || message.wParam == VK_RWIN )
				{
					if( message.message == WM_KEYUP )
					{
						inputManager->ReleaseMouse();
						bRecapture = TRUE; // This means, we want to recapture the mouse if the window is clicked back on
					}

					bHandled = true;
				}
				else
				{
					bHandled = HandleKeyboardInput( message, *inputManager );
				}
			}
			else if( message.message == WM_QUIT )
			{
				break;
			}

			// If the engine didnt handle this message, pass it along to the OS to process
			if( !bHandled )
			{
				TranslateMessage( &message );
				DispatchMessage( &message );
			}
		}
	}

	// We have exited the main message loop, so the engine is closing
	inputManager->ReleaseMouse();

	return 0;
}


void Impl_Shutdown()
{
	auto eng = Hyperion::Engine::Get();
	if( !eng )
	{
		MessageBox( NULL, L"Failed to properly shutdown engine, couldnt get instance during shutdown function", L"Hyperion Shutdown Error", MB_OK );
		return;
	}

	eng->Stop();
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
		}
		break;

	case WM_DESTROY:

		PostQuitMessage( 0 );
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


bool HandleMouseInput( MSG& inMsg, Hyperion::InputManager& inManager )
{
	bool bHandled = false;

	if( inMsg.message == WM_MOUSEMOVE )
	{
		if( inManager.IsMouseCaptured() )
		{
			// Move mouse to capture position
			CenterCursor();
		}
		else
		{
			// Figure out how much the mouse has moved since the last input
			LONG MouseX = GET_X_LPARAM( inMsg.lParam );
			LONG MouseY = GET_Y_LPARAM( inMsg.lParam );

			LONG DeltaX = MouseX - pCursorCenter.x;
			LONG DeltaY = MouseY - pCursorCenter.y;

			// Dispatch mouse moved event, and update the position where the cursor will be set on re-capture
			if( DeltaX != 0L )
			{
				inManager.HandleAxisInput( Hyperion::InputAxis::MouseX, DeltaX, MouseX );
				pCursorCenter.x = MouseX;
			}

			if( DeltaY != 0L )
			{
				inManager.HandleAxisInput( Hyperion::InputAxis::MouseY, DeltaY, MouseY );
				pCursorCenter.y = MouseY;
			}
		}
	}
	else
	{
		// Check if we should recapture the window
		if( inMsg.message == WM_LBUTTONDOWN && bRecapture )
		{
			bRecapture	= FALSE;
			bHandled	= true;

			inManager.CaptureMouse();
		}
		else
		{
			// Translate and dispatch key press event for mouse
			bool bIsPress;
			auto MouseButton = TranslateMouseButton( inMsg, bIsPress );

			if( MouseButton != Hyperion::Keys::NONE )
			{
				if( bIsPress )
				{
					inManager.HandleKeyPress( MouseButton );
				}
				else
				{
					inManager.HandleKeyRelease( MouseButton );
				}

				bHandled = true;
			}
		}
	}

	// Ensure mouse is limited to screen bounds while captured
	if( inManager.IsMouseCaptured() && hWindow )
	{
		RECT windowPos;
		GetWindowRect( hWindow, &windowPos );
		ClipCursor( &windowPos );
	}

	return bHandled;
}


bool HandleKeyboardInput( MSG& inMsg, Hyperion::InputManager& inManager )
{
	// Translate keypress, determine if it was pressed or released, then dispatch the input event
	bool bWasPress = false;
	auto PressedKey = TranslateKeyboardButton( inMsg, bWasPress );

	if( PressedKey == Hyperion::Keys::NONE )
	{
		#ifdef HYPERION_DEBUG
		Hyperion::Console::WriteLine( "[WARNING] Win32: Unknown keyboard button press! '", inMsg.wParam, "'" );
		#endif

		return false;
	}

	if( inMsg.message == WM_KEYDOWN )
	{
		inManager.HandleKeyPress( PressedKey );
		return true;
	}

	if( inMsg.message == WM_KEYUP )
	{
		inManager.HandleKeyRelease( PressedKey );
		return true;
	}

	return false;
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

		CenterCursor();
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


Hyperion::Keys TranslateMouseButton( MSG& inMessage, bool& outWasPress )
{
	outWasPress = false;

	switch( inMessage.message )
	{
	case WM_LBUTTONDOWN: // Fall through intentional
		outWasPress = true;
		[[fallthrough]];
	case WM_LBUTTONUP:
		return Hyperion::Keys::MOUSE1;
	case WM_RBUTTONDOWN:
		outWasPress = true;
		[[fallthrough]];
	case WM_RBUTTONUP:
		return Hyperion::Keys::MOUSE2;
	case WM_MBUTTONDOWN:
		outWasPress = true;
		[[fallthrough]];
	case WM_MBUTTONUP:
		return Hyperion::Keys::MOUSE3;
	case WM_XBUTTONDOWN:
		outWasPress = true;
		[[fallthrough]];
	case WM_XBUTTONUP:

		switch( HIWORD( inMessage.wParam ) )
		{
		case XBUTTON1:
			return Hyperion::Keys::MOUSE4;
		case XBUTTON2:
			return Hyperion::Keys::MOUSE5;
		}

	default:
		return Hyperion::Keys::NONE;
	}
}


Hyperion::Keys TranslateKeyboardButton( MSG& inMessage, bool& outWasPress )
{
	// TODO TODO TODO:
	// Create an array, where the index is the keycode value from windows, and the value is the hyperion key code
	outWasPress = false;
	return Hyperion::Keys::NONE;
}