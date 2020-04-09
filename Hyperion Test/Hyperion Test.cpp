// Hyperion Test.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Hyperion Test.h"
#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/GameManager.h"
#include "Hyperion/Core/ThreadManager.h"
#include "Hyperion/Core/RenderManager.h"
#include "Hyperion/Console/Console.h"
#include "Hyperion/File/UnifiedFileSystem.h"
#include "Tests.hpp"

#include <windowsx.h>
#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <fstream>
#include <excpt.h>



#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hWindow;
POINT pLastCursorPos;
BOOL bIsCaptured = FALSE;
BOOL bRecapture = FALSE;
POINT pCursorCenterPos;


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass( HINSTANCE hInstance );
BOOL                InitInstance( HINSTANCE, int, UINT32, UINT32 );
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK    About( HWND, UINT, WPARAM, LPARAM );
void BindCrtHandlesToStdHandles( bool, bool, bool );
bool HandleMouseInput( MSG&, Hyperion::InputManager& );
bool HandleKeyboardInput( MSG&, Hyperion::InputManager& );
void OnMouseCapture( bool );
void CenterCursor();
Hyperion::Keys TranslateMouseButton( MSG&, bool& );
Hyperion::Keys TranslateKeyboardButton( MSG& );



int Impl_Main( HINSTANCE hInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Process command line arguments
    std::vector< Hyperion::String > args;

    int argCount = 0;
    LPWSTR* szArgList = CommandLineToArgvW( lpCmdLine, &argCount );
    if( szArgList != NULL )
    {
        for( int i = 0; i < argCount; i++ )
        {
            args.emplace_back( szArgList[ i ], Hyperion::StringEncoding::UTF16 );
        }
    }

    // Now, check for flags
    bool bHasOSConsoleFlag = false;
    for( auto& flag : args )
    {
        if( flag.Equals( "-os_console" ) )
        {
            bHasOSConsoleFlag = true;
        }
    }

    // If we have an OS console.. then we need to create the window
    if( bHasOSConsoleFlag )
    {
        AllocConsole();
        BindCrtHandlesToStdHandles( true, true, true );
    }

    // Begin engine initialization
    // First, we need to startup the file system
    Hyperion::UnifiedFileSystem::Initialize();

    // Next, we need to start the console
    if( !Hyperion::Console::Start( Hyperion::FLAG_CONSOLE_OS_OUTPUT ) )
    {
        MessageBox( hWindow, L"Hyperion engine failed to initialize! Couldnt start console!", L"Hyperion Error!", MB_OK );
        return -1;
    }

    Hyperion::uint32 ifs = 0;

    // Check if were fullscreen or not
    Hyperion::Console::GetVar< Hyperion::uint32 >( "r_fullscreen", ifs );

    // Get the screen resolution
    Hyperion::String resStr;
    Hyperion::ScreenResolution res;
    res.Width = 0;
    res.Height = 0;

    // Get the virtual screen size
    //int ScreenW = GetSystemMetrics( SM_CXVIRTUALSCREEN );
    //int ScreenH = GetSystemMetrics( SM_CYVIRTUALSCREEN );
    int ScreenW = 1080;
    int ScreenH = 720;

    if( Hyperion::Console::GetVarAsString( "r_resolution", resStr ) )
    {
        res = Hyperion::RenderManager::ReadResolution( resStr, ifs, false );
    }

    // First, check for invalid resolution, and update the console value to a decent default (screen size)
    if( res.Width < 480 || res.Height < 360 )
    {
        res.Width = ScreenW;
        res.Height = ScreenH;

        Hyperion::Console::SetVar( "r_resolution", Hyperion::ToString( res.Width ) + "," + Hyperion::ToString( res.Height ), false );
        Hyperion::Console::WriteLine( "[Warning] Win32: Invalid resolution selected.. defaulting to screen size (", res.Width, ", ", res.Height, ")" );
    }

    // Next, we need to determine the size of the window
    // For Fullscreen: The size of the screen, regardless of selected resolution
    // For Windowed: The selected resolution
    UINT windowWidth;
    UINT windowHeight;

    if( ifs == 0 )
    {
        // Windowed Mode
        windowWidth = res.Width;
        windowHeight = res.Height;
    }
    else
    {
        // Fullscreen Mode
        windowWidth = ScreenW;
        windowHeight = ScreenH;
    }

    Hyperion::Console::WriteLine( "[State] Win32: Creating game window.. Resolution = ", res.Width, "x", res.Height, " Window Size = ", windowWidth, "x", windowHeight, " Fullscreen? ", ( ifs == 0 ) ? "false" : "true" );


    // Initialize global strings
    LoadStringW( hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING );
    LoadStringW( hInstance, IDC_HYPERIONTEST, szWindowClass, MAX_LOADSTRING );
    MyRegisterClass( hInstance );

    // Perform application initialization:
    if( !InitInstance( hInstance, nCmdShow, windowWidth, windowHeight ) || !hWindow )
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDC_HYPERIONTEST ) );

    // TODO: Feed console any command line arguments

    // Next, we startup the threading system
    if( !Hyperion::ThreadManager::Start( 0 ) )
    {
        MessageBox( hWindow, L"Hyperion engine failed to initialize! Couldnt start thread manager!", L"Hyperion Error!", MB_OK );
        return -1;
    }

    Hyperion::IRenderOutput outputWindow;
    outputWindow.Value = hWindow;

    // Now, we can start our renderer
    // This is a blocking call, we actually wait for API initialization 
    if( !Hyperion::RenderManager::Start( outputWindow, 0 ) )
    {
        MessageBox( hWindow, L"Hyperion engine failed to initialize! Couldnt start the renderer!", L"Hyperion Error!", MB_OK );
        return -1;
    }

    // Finally, start up the game instance
    // TODO: Declare custom instance type here
    auto instFactory = std::make_shared< Hyperion::InstanceFactory< Hyperion::GameInstance > >();
    if( !Hyperion::GameManager::Start( 0, instFactory ) )
    {
        MessageBox( hWindow, L"Hyperion engine failed to initialize! Couldnt start the game instance!", L"Hyperion Error!", MB_OK );
        return -1;
    }

    // Get reference to the input manager
    Hyperion::InputManager& im = Hyperion::GameManager::GetInputManager();

    // Set callback for when mouse capture is changed
    im.SetCaptureCallback( &OnMouseCapture );
    im.CaptureMouse();

    // Create task for our test function
    Hyperion::ThreadManager::CreateTask< void >( RunEngineTests, Hyperion::THREAD_GAME );
    Hyperion::ThreadManager::CreateTask< void >( RunMiscTests );

    MSG msg;

    // Main message loop:
    while( GetMessage( &msg, nullptr, 0, 0 ) )
    {
        if( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) )
        {
            // Check for user input
            bool bHandled = false;
            if( msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST )
            {
                bHandled = HandleMouseInput( msg, im );
            }
            else if( msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST )
            {
                // Check if we clicked the windows button, so we can release the cursor
                if( msg.wParam == VK_LWIN || msg.wParam == VK_RWIN )
                {
                    if( msg.message == WM_KEYUP )
                    {
                        im.ReleaseMouse();
                        bRecapture = TRUE; // This means, we want to recapture the mouse if the window is clicked back on
                    }

                    bHandled = true;
                }
                else
                {
                    bHandled = HandleKeyboardInput( msg, im );
                }
            }
            else if( msg.message == WM_QUIT )
            {
                break;
            }

            // If the engine didnt handle this message, pass it along to the OS to process
            if( !bHandled )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        }
    }

    // Release mouse before we start to shutdown
    im.ReleaseMouse();

    // Now, that the application is shutting down, we need to shut down each of our systems
    Hyperion::GameManager::Stop();
    Hyperion::RenderManager::Stop();
    Hyperion::ThreadManager::Stop();
    Hyperion::Console::Stop();
    Hyperion::UnifiedFileSystem::Shutdown();

    // If we created a console window, we need to free it
    if( bHasOSConsoleFlag )
    {
        FreeConsole();
    }

    return (int) msg.wParam;
}


int HandleCrash( EXCEPTION_POINTERS* Exceptions )
{
    return EXCEPTION_EXECUTE_HANDLER;
}


int Impl_MainWrapper( HINSTANCE hInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    int ErrRet = 0;

 #if _WIN64
    __try
 #endif
    {
        ErrRet = Impl_Main( hInstance, lpCmdLine, nCmdShow );
    }
#if _WIN64
    __except( HandleCrash( GetExceptionInformation() ), EXCEPTION_CONTINUE_SEARCH )
    {
        (void) ( 0 );
    }
#endif

    return ErrRet;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    int ErrRet = 0;

    __try
    {
        ErrRet = Impl_Main( hInstance, lpCmdLine, nCmdShow );
    }
#if _WIN64
    __except( EXCEPTION_EXECUTE_HANDLER )
#else
    __except( HandleCrash( GetExceptionInformation() ) )
#endif
    {
        // TODO: Better handling
        MessageBoxW( NULL, L"Hyperion Engine Crash!", L"There was an uncaught exception", MB_OK );
    }

    // Ensure engine is fully shutdown, if there was an exception thrown, shutdown might have not happened
    Hyperion::GameManager::GetInputManager().ReleaseMouse();

    Hyperion::GameManager::Stop();
    Hyperion::RenderManager::Stop();
    Hyperion::ThreadManager::Stop();
    Hyperion::Console::Stop();
    Hyperion::UnifiedFileSystem::Shutdown();

    return ErrRet;
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
        pCursorCenterPos = centerPoint;
    }

}


void OnMouseCapture( bool bCapture )
{
    if( bCapture )
    {
        // If were already captured, then do nothing
        // Also, if we dont have a window handle, then do nothing
        if( bIsCaptured || !hWindow )
        {
            return;
        }

        // Center the cursor
        CenterCursor();

        // Clip mouse to the bounding box of the application
        RECT WindowPosition;
        GetWindowRect( hWindow, &WindowPosition );

        ClipCursor( &WindowPosition );
        bIsCaptured = true;

        // Hide Cursor
        while( ShowCursor( FALSE ) > 0 );
    }
    else
    {
        // If were not captured then do nothing
        if( !bIsCaptured )
        {
            return;
        }

        // Restore cursor position
        if( !SetCursorPos( pLastCursorPos.x, pLastCursorPos.y ) )
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
            pCursorCenterPos = screenSpacePoint;
        }
    }
}


bool HandleMouseInput( MSG& Input, Hyperion::InputManager& im )
{
    bool bHandled = false;

    if( Input.message == WM_MOUSEMOVE )
    {
        // TODO:
        // Ignore this if were not captured?

        // Handle mouse movement
        LONG MouseX = GET_X_LPARAM( Input.lParam );
        LONG MouseY = GET_Y_LPARAM( Input.lParam );

        LONG DeltaX = MouseX - pCursorCenterPos.x;
        LONG DeltaY = MouseY - pCursorCenterPos.y;

        // Dispatch mouse event
        im.HandleAxisInput( Hyperion::InputAxis::MouseX, DeltaX, MouseX );
        im.HandleAxisInput( Hyperion::InputAxis::MouseY, DeltaY, MouseY );

        // Recenter the cursor on the screen
        if( im.IsMouseCaptured() )
        {
            CenterCursor();
        }
        else
        {
            pCursorCenterPos.x = MouseX;
            pCursorCenterPos.y = MouseY;
        }
    }
    else
    {
        // Check if we should recapture the window
        if( Input.message == WM_LBUTTONDOWN && bRecapture )
        {
            bRecapture = FALSE;
            bHandled = true;

            im.CaptureMouse();
        }
        else
        {
            // Translate and dispatch key press event
            bool bIsPress;
            auto MouseButton = TranslateMouseButton( Input, bIsPress );

            if( MouseButton != Hyperion::Keys::NONE )
            {
                if( bIsPress )
                {
                    im.HandleKeyPress( MouseButton );
                }
                else
                {
                    im.HandleKeyRelease( MouseButton );
                }

                bHandled = true;
            }
        }
    }

    // Ensure mouse is clipped to window bounds
    if( im.IsMouseCaptured() && hWindow )
    {
        RECT WindowPosition;
        GetWindowRect( hWindow, &WindowPosition );
        ClipCursor( &WindowPosition );
    }

    return bHandled;
}


bool HandleKeyboardInput( MSG& Input, Hyperion::InputManager& im )
{
    // Determine which key was pressed
    auto PressedKey = TranslateKeyboardButton( Input );
    if( PressedKey == Hyperion::Keys::NONE )
    {
        Hyperion::Console::WriteLine( "[WARNING] Win32: Unknown keyboard button press! '", Input.wParam, "'" );
        return false;
    }

    if( Input.message == WM_KEYDOWN )
    {
        im.HandleKeyPress( PressedKey );
        return true;
    }

    if( Input.message == WM_KEYUP )
    {
        im.HandleKeyRelease( PressedKey );
        return true;
    }

    return false;
}


Hyperion::Keys TranslateMouseButton( MSG& Message, bool& outWasPressed )
{
    outWasPressed = false;
    
    switch( Message.message )
    {
    case WM_LBUTTONDOWN:
        outWasPressed = true;
    case WM_LBUTTONUP:
        return Hyperion::Keys::MOUSE1;
    case WM_RBUTTONDOWN:
        outWasPressed = true;
    case WM_RBUTTONUP:
        return Hyperion::Keys::MOUSE2;
    case WM_MBUTTONDOWN:
        outWasPressed = true;
    case WM_MBUTTONUP:
        return Hyperion::Keys::MOUSE3;
    case WM_XBUTTONDOWN:
        outWasPressed = true;
    case WM_XBUTTONUP:

        switch( HIWORD( Message.wParam ) )
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


Hyperion::Keys TranslateKeyboardButton( MSG& inMessage )
{
    // For now.. this is not implemeneted.. we need to write out a 
    // Win32 Keycode -> Hyperion Keycode  lookup table

    return Hyperion::Keys::NONE;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HYPERIONTEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_HYPERIONTEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance( HINSTANCE hInstance, int nCmdShow, UINT32 Width, UINT32 Height )
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW( szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, Width, Height, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   // Store window handle in global value as well
   hWindow = hWnd;

   ShowWindow( hWnd, nCmdShow );
   UpdateWindow( hWnd );

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


void BindCrtHandlesToStdHandles( bool bindStdIn, bool bindStdOut, bool bindStdErr )
{

    if( bindStdIn )
    {
        // Not implemented
    }

    if( bindStdOut )
    {
        FILE* dummyFile;
        freopen_s( &dummyFile, "nul", "w", stdout );
    }

    if( bindStdErr )
    {
        FILE* dummyFile;
        freopen_s( &dummyFile, "nul", "w", stderr );
    }

    if( bindStdIn )
    {
        // Not implemented
    }

    if( bindStdOut )
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

    if( bindStdErr )
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

    if( bindStdIn )
    {
        // Not implemented
    }

    if( bindStdOut )
    {
        std::wcout.clear();
        std::cout.clear();
    }

    if( bindStdErr )
    {
        std::wcerr.clear();
        std::cerr.clear();
    }

}