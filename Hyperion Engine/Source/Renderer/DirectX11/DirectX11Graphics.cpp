/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/DirectX11Graphics.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DirectX11/DirectX11Graphics.h"
#include "Hyperion/Renderer/DirectX11/DirectX11Buffer.h"
#include "Hyperion/Renderer/DirectX11/DirectX11Texture.h"
#include "Hyperion/Renderer/DirectX11/DirectX11RenderTarget.h"
#include "Hyperion/Renderer/DirectX11/Shaders/DirectX11GBufferShader.h"
#include "Hyperion/Renderer/DirectX11/Shaders/DirectX11ForwardShader.h"
#include "Hyperion/Renderer/DirectX11/Shaders/DirectX11LightingShader.h"
#include "Hyperion/Library/Math/Geometry.h"
#include "Hyperion/Renderer/DirectX11/DirectX11DepthStencil.h"
#include "Hyperion/Renderer/GBuffer.h"


namespace Hyperion
{

	/*------------------------------------------------------------------------------------------
		DirectX11Graphics Constructor 
	------------------------------------------------------------------------------------------*/
	DirectX11Graphics::DirectX11Graphics()
		: m_bRunning( false ), m_bVSync( false ), m_Resolution(), m_bDepthEnabled( true )
	{

	}


	/*------------------------------------------------------------------------------------------
		DirectX11Graphics Destructor 
	------------------------------------------------------------------------------------------*/
	DirectX11Graphics::~DirectX11Graphics()
	{
		Shutdown();
	}


	/*------------------------------------------------------------------------------------------
		DirectX11Graphics::SetResolution 
	------------------------------------------------------------------------------------------*/
	bool DirectX11Graphics::SetResolution( const ScreenResolution& inResolution )
	{
		// If were running.. perform update on the fly
		if( m_bRunning && m_SwapChain )
		{
			// Get output device
			IDXGIOutput* output = nullptr;
			if( FAILED( m_SwapChain->GetContainingOutput( &output ) ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to get reoslution list.. couldnt get output device" );
				return false;
			}

			// Get list of supported display modes on this output device
			uint32 numberModes = 0;
			if( FAILED( output->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numberModes, NULL ) ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to get reoslution list.. couldnt get display modes list" );
				output->Release();
				return false;
			}

			// Create array of supported modes
			DXGI_MODE_DESC* supportedModes = new DXGI_MODE_DESC[ numberModes ];
			ZeroMemory( supportedModes, sizeof( DXGI_MODE_DESC ) * numberModes );

			// Fill out this array
			if( FAILED( output->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numberModes, supportedModes ) ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to get resolution list.. couldnt fill display modes list!" );
				output->Release();
				delete[] supportedModes;
				return false;
			}

			// Release output device
			output->Release();

			// Now, we want to find a mode with matching resolution
			DXGI_MODE_DESC targetMode;
			bool bFoundMode = false;

			for( uint32 i = 0; i < numberModes; i++ )
			{
				if( inResolution.Width == supportedModes[ i ].Width &&
					inResolution.Height == supportedModes[ i ].Height )
				{
					targetMode = supportedModes[ i ];
					bFoundMode = true;
					break;
				}
			}

			if( !bFoundMode )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to update resolution.. selected resolution of ", inResolution.Width, "x", inResolution.Height, " is invalid!" );
				return false;
			}

			m_Resolution = inResolution;

			// Update to our new resolution
			BOOL currentlyFullScreen = FALSE;
			m_SwapChain->GetFullscreenState( &currentlyFullScreen, NULL );

			// Zero out the refresh rate
			targetMode.RefreshRate.Numerator	= 0;
			targetMode.RefreshRate.Denominator	= 0;

			if( currentlyFullScreen != (BOOL)inResolution.FullScreen )
			{
				if( inResolution.FullScreen )
				{
					// Set to fullscreen

					// Resize our swap chain target
					if( FAILED( m_SwapChain->ResizeTarget( &targetMode ) ) )
					{
						Console::WriteLine( "[ERROR] DX11Renderer: Failed to update reoslution.. couldnt resize swap chain target" );
						return false;
					}

					// Set swap chain to fullscreen
					if( FAILED( m_SwapChain->SetFullscreenState( TRUE, NULL ) ) )
					{
						Console::WriteLine( "[ERROR] DX11Renderer: Failed to update reoslution.. couldnt set fullscreen state" );
						return false;
					}
				}
				else
				{
					// Disable fullscreen
					if( FAILED( m_SwapChain->SetFullscreenState( FALSE, NULL ) ) )
					{
						Console::WriteLine( "[ERROR] DX11Renderer: Failed to update resolution.. couldnt exit fullscreen mode" );
						return false;
					}

					// Set window size to our resolution
					RECT newSize = { 0, 0, (long) targetMode.Width, (long) targetMode.Height };
					if( !AdjustWindowRectEx( &newSize, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW ) )
					{
						Console::WriteLine( "[ERROR] DX11Renderer: Failed to update resolution.. couldnt resize window when exiting fullscreen" );
						return false;
					}

					// Center the window
					SetWindowPos( m_Output, HWND_TOP, 0, 0, newSize.right - newSize.left, newSize.bottom - newSize.top, SWP_NOMOVE );
				}
			}

			// Resize the target
			if( FAILED( m_SwapChain->ResizeTarget( &targetMode ) ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to update reoslution.. couldnt resize swap chain target" );
				return false;
			}

			// Reset the render target view and depth stencil view.. we need to recreate with updated sizes
			m_RenderTarget.reset();
			m_BackBuffer.reset();

			// Resize the swap chain buffers
			if( FAILED( m_SwapChain->ResizeBuffers( 0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ) ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to update resolution.. couldnt update swap chain buffer size" );
				return false;
			}

			if( FAILED( m_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)m_BackBuffer->GetAddress() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to update resolution.. coudlnt recreate back buffer" );
				return false;
			}

			if( FAILED( m_Device->CreateRenderTargetView( m_BackBuffer->Get(), NULL, m_RenderTarget->GetAddress() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to update resolution.. couldnt create render target view" );
				return false;
			}

			// Create Depth Buffer
			m_DepthStencil->Resize( m_Device.Get(), targetMode.Width, targetMode.Height );

			// Set the correct depth stencil state
			m_DeviceContext->OMSetDepthStencilState( m_bDepthEnabled ? m_DepthStencilState.Get() : m_DepthDisabledState.Get(), 1 );

			// Bind render target view and depth stencil view to the output render view
			ID3D11RenderTargetView *aRenderViews[ 1 ] = { m_RenderTarget->Get() }; // array of pointers
			m_DeviceContext->OMSetRenderTargets( 1, aRenderViews, m_DepthStencil->GetStencilView() );

			// Now update the viewport
			D3D11_VIEWPORT viewport{};
			viewport.Height		= (FLOAT)targetMode.Height;
			viewport.Width		= (FLOAT)targetMode.Width;
			viewport.MinDepth	= 0.f;
			viewport.MaxDepth	= 1.f;
			viewport.TopLeftX	= 0.f;
			viewport.TopLeftY	= 1.f;

			// Set viewport
			m_DeviceContext->RSSetViewports( 1, &viewport );

			// Update matricies
			GenerateMatricies( inResolution, DirectX::XM_PIDIV4, SCREEN_NEAR, SCREEN_FAR );
			GenerateScreenGeometry( inResolution.Width, inResolution.Height );

			return true;
		}
		else
		{
			// Were not running, so just store the resolution for when we startup
			m_Resolution = inResolution;
			return true;
		}
	}


	/*------------------------------------------------------------------------------------------
		DirectX11Graphics::GetAvailableResolutions 
	------------------------------------------------------------------------------------------*/
	std::vector< ScreenResolution > DirectX11Graphics::GetAvailableResolutions()
	{
		std::vector< ScreenResolution > outputList;

		if( m_bRunning && m_SwapChain )
		{
			// Get output device
			IDXGIOutput* output = nullptr;
			if( FAILED( m_SwapChain->GetContainingOutput( &output ) ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to get reoslution list.. couldnt get output device" );
				return outputList;
			}

			// Get list of supported display modes on this output device
			uint32 numberModes = 0;
			if( FAILED( output->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numberModes, NULL ) ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to get reoslution list.. couldnt get display modes list" );
				output->Release();
				return outputList;
			}

			// Create array of supported modes
			DXGI_MODE_DESC* supportedModes = new DXGI_MODE_DESC[ numberModes ];
			ZeroMemory( supportedModes, sizeof( DXGI_MODE_DESC ) * numberModes );

			// Fill out this array
			if( FAILED( output->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numberModes, supportedModes ) ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to get resolution list.. couldnt fill display modes list!" );
				output->Release();
				delete[] supportedModes;
				return outputList;
			}

			// Release output device
			output->Release();

			// Build list of supported resolutions
			for( uint32 i = 0; i < numberModes; i++ )
			{
				ScreenResolution res;

				res.Width		= supportedModes[ i ].Width;
				res.Height		= supportedModes[ i ].Height;
				res.FullScreen	= true;

				outputList.push_back( res );
			}
		}

		return outputList;
	}


	/*------------------------------------------------------------------------------------------
		DirectX11Graphics::SetVSync 
	------------------------------------------------------------------------------------------*/
	void DirectX11Graphics::SetVSync( bool bVsync )
	{
		m_bVSync = bVsync;
	}


	/*------------------------------------------------------------------------------------------
		DirectX11Graphics::Initialize 
	------------------------------------------------------------------------------------------*/
	bool DirectX11Graphics::Initialize( void* pWindow )
	{
		Console::WriteLine( "[STATUS] DX11: Initializing..." );

		if( !pWindow )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to initialize.. output window invalid!" );
			return false;
		}

		HWND windowHandle = static_cast< HWND >( pWindow );

		// If any resources are still open.. shut them down
		ShutdownResources();

		// Initialize our resoource using the parameters we have set
		if( !InitializeResources( windowHandle, m_Resolution ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to initialize.. resources couldnt be initialized..." );
			return false;
		}

		m_Output = windowHandle;

		// Generate view matricies based on resolution
		GenerateMatricies( m_Resolution, DirectX::XM_PIDIV4, SCREEN_NEAR, SCREEN_FAR );

		// Set default view position and matrix
		ViewState defaultView;

		defaultView.Position	= Vector3D( 0.f, 0.f, 0.f );
		defaultView.Rotation	= Angle3D( 0.f, 0.f, 0.f );
		defaultView.FOV			= DirectX::XM_PIDIV4;

		SetCameraInfo( defaultView );

		m_bRunning = true;
		return true;
	}


	/*------------------------------------------------------------------------------------------
		DirectX11Graphics::Shutdown 
	------------------------------------------------------------------------------------------*/
	void DirectX11Graphics::Shutdown()
	{
		if( !m_bRunning )
			return;

		Console::WriteLine( "[STATUS] DX11: Shutting down..." );

		// Shutdown any open resources
		ShutdownResources();
		m_bRunning = false;
	}


	bool DirectX11Graphics::IsRunning() const
	{
		return m_bRunning;
	}


	/*------------------------------------------------------------------------------------------
		DirectX11Graphics::InitializeResources 
	------------------------------------------------------------------------------------------*/
	bool DirectX11Graphics::InitializeResources( HWND Target, ScreenResolution& Resolution )
	{
		if( !Target ) return false;

		// Create pointers to dxgi resources we will create in our try block
		IDXGIFactory* ptrFactory			= nullptr;
		IDXGIAdapter* ptrAdapter			= nullptr;
		IDXGIOutput* ptrOutput				= nullptr;
		DXGI_MODE_DESC* ptrDisplayModes		= nullptr;
		
		try
		{
			// Create graphics interface factory
			HRESULT res = CreateDXGIFactory( __uuidof( IDXGIFactory ), (void**) &ptrFactory );
			if( FAILED( res ) || !ptrFactory )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create graphics interface factory" );
				throw std::exception();
			}

			// Create video device adapter
			res = ptrFactory->EnumAdapters( 0, &ptrAdapter );
			if( FAILED( res ) || !ptrAdapter )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt enumerate adapters" );
				throw std::exception();
			}

			res = ptrAdapter->EnumOutputs( 0, &ptrOutput );
			if( FAILED( res ) || !ptrOutput )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt enum outputs!" );
				throw std::exception();
			}

			unsigned int numModes = 0;
			res = ptrOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt get display modes list" );
				throw std::exception();
			}

			ptrDisplayModes = new DXGI_MODE_DESC[ numModes ];
			res = ptrOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, ptrDisplayModes );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt fill display mode list" );
				throw std::exception();
			}

			if( numModes <= 0 )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: There are no supported display modes! Failed to initialize" );
				throw std::exception();
			}

			// Now that we have a list of supported resolutions.. lets check if the desired one is supported
			bool bResolutionSupported = false;
			DXGI_MODE_DESC selectedMode;

			for( uint32 i = 0; i < numModes; i++ )
			{
				if( ptrDisplayModes[ i ].Width == Resolution.Width &&
					ptrDisplayModes[ i ].Height == Resolution.Height )
				{
					bResolutionSupported = true;
					selectedMode = ptrDisplayModes[ i ];
				}
			}

			if( !bResolutionSupported )
			{
				// We need to default to one of the modes in the list
				Console::WriteLine( "[WARNING] DX11Renderer: Selected resolution (", Resolution.Width, "x", Resolution.Height, ") wasnt supported.. defaulting!" );
				selectedMode = ptrDisplayModes[ numModes - 1 ];
				Console::WriteLine( "[WARNING] DX11Renderer: Defaulting to resolution (", selectedMode.Width, "x", selectedMode.Height, ")" );
			}

			// Update stored resolution to the supported one
			Resolution.Width		= selectedMode.Width;
			Resolution.Height		= selectedMode.Height;

			// Get video card description
			DXGI_ADAPTER_DESC videoCardDescription;
			ZeroMemory( &videoCardDescription, sizeof( videoCardDescription ) );

			res = ptrAdapter->GetDesc( &videoCardDescription );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. coudlnt read video card information" );
				throw std::exception();
			}

			m_GraphicsMemory	= (uint32)( videoCardDescription.DedicatedVideoMemory / 1024 / 1024 );
			m_GraphicsDevice	= String( std::wstring( videoCardDescription.Description ), StringEncoding::UTF16 );

			// Release everything we dont need anymore
			delete[] ptrDisplayModes;
			ptrDisplayModes = nullptr;

			ptrOutput->Release();
			ptrOutput = nullptr;

			ptrAdapter->Release();
			ptrAdapter = nullptr;

			ptrFactory->Release();
			ptrFactory = nullptr;

			// Resize window to match our target resolution
			RECT newSize = { 0, 0, (long) Resolution.Width, (long) Resolution.Height };
			if( !AdjustWindowRectEx( &newSize, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to resize window during initialization!\n" );
				throw std::exception();
			}

			// Center the window
			SetWindowPos( m_Output, HWND_TOP, 0, 0, newSize.right - newSize.left, newSize.bottom - newSize.top, SWP_NOMOVE );

			// Create swap chain
			DXGI_SWAP_CHAIN_DESC swapChainDesc;
			ZeroMemory( &swapChainDesc, sizeof( swapChainDesc ) );

			// DEBUD SET BACK TO 2
			swapChainDesc.BufferCount			= 1;
			swapChainDesc.BufferDesc.Width		= selectedMode.Width;
			swapChainDesc.BufferDesc.Height		= selectedMode.Height;
			swapChainDesc.BufferDesc.Format		= DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.OutputWindow			= Target;
			swapChainDesc.SampleDesc.Count		= 1;
			swapChainDesc.SampleDesc.Quality	= 0;
			swapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT_DISCARD; // DEB SET BACZK
			swapChainDesc.Flags					= 0; // DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			swapChainDesc.Windowed				= !Resolution.FullScreen;

			swapChainDesc.BufferDesc.RefreshRate.Numerator = m_bVSync ? selectedMode.RefreshRate.Numerator : 0;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = m_bVSync ? selectedMode.RefreshRate.Denominator : 1;
			
			// Create swap chain, device and context
			D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;

#ifdef HYPERION_DEBUG_RENDERER
			res = D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1,
				D3D11_SDK_VERSION, &swapChainDesc, m_SwapChain.GetAddressOf(), m_Device.GetAddressOf(), NULL, m_DeviceContext.GetAddressOf() );
#else
			res = D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1, 
					D3D11_SDK_VERSION, &swapChainDesc, m_SwapChain.GetAddressOf(), m_Device.GetAddressOf(), NULL, m_DeviceContext.GetAddressOf() );
#endif

			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create swap chain and device!" );
				throw std::exception();
			}

			// Get back buffer texture
			m_BackBuffer = std::shared_ptr< DirectX11Texture2D >( new DirectX11Texture2D() );
			res = m_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**) m_BackBuffer->GetAddress() );

			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt get pointer to the back buffe" );
				throw std::exception();
			}

			// Create render target view
			m_RenderTarget = std::shared_ptr< DirectX11RenderTarget >( new DirectX11RenderTarget( m_BackBuffer, m_Device ) );
			res = m_Device->CreateRenderTargetView( m_BackBuffer->Get(), NULL, m_RenderTarget->GetAddress() );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create render target view" );
				throw std::exception();
			}

			// Create common states object
			//m_CommonStates = std::make_unique< DirectX::CommonStates >( m_Device.Get() );

			// Create depth stencil state
			D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
			ZeroMemory( &depthStencilDesc, sizeof( depthStencilDesc ) );

			depthStencilDesc.DepthEnable		= true;
			depthStencilDesc.DepthWriteMask		= D3D11_DEPTH_WRITE_MASK_ALL;
			depthStencilDesc.DepthFunc			= D3D11_COMPARISON_LESS;
			depthStencilDesc.StencilEnable		= true;
			depthStencilDesc.StencilReadMask	= 0xFF;
			depthStencilDesc.StencilWriteMask	= 0xFF;

			depthStencilDesc.FrontFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
			depthStencilDesc.FrontFace.StencilDepthFailOp	= D3D11_STENCIL_OP_INCR;
			depthStencilDesc.FrontFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
			depthStencilDesc.FrontFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;

			depthStencilDesc.BackFace.StencilFailOp			= D3D11_STENCIL_OP_KEEP;
			depthStencilDesc.BackFace.StencilDepthFailOp	= D3D11_STENCIL_OP_DECR;
			depthStencilDesc.BackFace.StencilPassOp			= D3D11_STENCIL_OP_KEEP;
			depthStencilDesc.BackFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;

			res = m_Device->CreateDepthStencilState( &depthStencilDesc, m_DepthStencilState.GetAddressOf() );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create depth stencil state" );
				throw std::exception();
			}

			// Set active stencil state
			m_bDepthEnabled = true;
			m_DeviceContext->OMSetDepthStencilState( m_DepthStencilState.Get(), 1 );

			// Create Depth Stencil View
			m_DepthStencil = std::dynamic_pointer_cast<DirectX11DepthStencil>( CreateDepthStencil( selectedMode.Width, selectedMode.Height ) );
			if( !m_DepthStencil || !m_DepthStencil->IsValid() )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create depth stencil" );
				throw std::exception();
			}

			// Bind render target view and depth stencil view to the output render view
			ID3D11RenderTargetView *aRenderViews[ 1 ] = { m_RenderTarget->Get() }; // array of pointers
			m_DeviceContext->OMSetRenderTargets( 1, aRenderViews, m_DepthStencil->GetStencilView() );

			// Create the rasterizer
			D3D11_RASTERIZER_DESC rasterDesc;
			ZeroMemory( &rasterDesc, sizeof( rasterDesc ) );

			rasterDesc.AntialiasedLineEnable	= false;
			rasterDesc.CullMode					= D3D11_CULL_NONE;
			rasterDesc.DepthBias				= 0;
			rasterDesc.DepthBiasClamp			= 0.f;
			rasterDesc.DepthClipEnable			= true;
			rasterDesc.FillMode					= D3D11_FILL_SOLID;
			rasterDesc.FrontCounterClockwise	= false;
			rasterDesc.MultisampleEnable		= false;
			rasterDesc.ScissorEnable			= false;
			rasterDesc.SlopeScaledDepthBias		= 0.f;

			res = m_Device->CreateRasterizerState( &rasterDesc, m_RasterizerState.GetAddressOf() );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create rasterizer" );
				throw std::exception();
			}

			// Set raster state 
			m_DeviceContext->RSSetState( m_RasterizerState.Get() );

			// Setup viewport
			D3D11_VIEWPORT viewport;
			viewport.Height		= (FLOAT)selectedMode.Height;
			viewport.Width		= (FLOAT)selectedMode.Width;
			viewport.MinDepth	= 0.f;
			viewport.MaxDepth	= 1.f;
			viewport.TopLeftX	= 0.f;
			viewport.TopLeftY	= 1.f;

			// Set viewport
			m_DeviceContext->RSSetViewports( 1, &viewport );

			// Create additional depth stencil for disabled depth
			D3D11_DEPTH_STENCIL_DESC depthDisabledDesc;
			ZeroMemory( &depthDisabledDesc, sizeof( depthDisabledDesc ) );

			depthDisabledDesc.DepthEnable					= false;
			depthDisabledDesc.DepthWriteMask				= D3D11_DEPTH_WRITE_MASK_ALL;
			depthDisabledDesc.DepthFunc						= D3D11_COMPARISON_LESS;
			depthDisabledDesc.StencilEnable					= true;
			depthDisabledDesc.StencilReadMask				= 0xFF;
			depthDisabledDesc.StencilWriteMask				= 0xFF;
			depthDisabledDesc.FrontFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
			depthDisabledDesc.FrontFace.StencilDepthFailOp	= D3D11_STENCIL_OP_INCR;
			depthDisabledDesc.FrontFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
			depthDisabledDesc.FrontFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;
			depthDisabledDesc.BackFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
			depthDisabledDesc.BackFace.StencilDepthFailOp	= D3D11_STENCIL_OP_DECR;
			depthDisabledDesc.BackFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
			depthDisabledDesc.BackFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;

			res = m_Device->CreateDepthStencilState( &depthDisabledDesc, m_DepthDisabledState.GetAddressOf() );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create depth disabled state" );
				throw std::exception();
			}

			// Create blend states
			D3D11_BLEND_DESC blendDesc;
			ZeroMemory( &blendDesc, sizeof( blendDesc ) );

			blendDesc.RenderTarget[ 0 ].BlendEnable				= TRUE;
			blendDesc.RenderTarget[ 0 ].SrcBlend				= D3D11_BLEND_ONE;
			blendDesc.RenderTarget[ 0 ].DestBlend				= D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[ 0 ].BlendOp					= D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[ 0 ].SrcBlendAlpha			= D3D11_BLEND_ONE;
			blendDesc.RenderTarget[ 0 ].DestBlendAlpha			= D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[ 0 ].BlendOpAlpha			= D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[ 0 ].RenderTargetWriteMask	= 0x0f;

			res = m_Device->CreateBlendState( &blendDesc, m_BlendState.GetAddressOf() );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create normal blend state" );
				throw std::exception();
			}

			blendDesc.RenderTarget[ 0 ].BlendEnable = FALSE;

			res = m_Device->CreateBlendState( &blendDesc, m_BlendDisabledState.GetAddressOf() );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create alpha disabled blend state" );
				throw std::exception();
			}

			// Set disabled blend state as default
			DisableAlphaBlending();

			// Create screen geometry
			GenerateScreenGeometry( selectedMode.Width, selectedMode.Height );

		}
		catch( ... )
		{
			if( ptrFactory ) { ptrFactory->Release(); ptrFactory = nullptr; }
			if( ptrAdapter ) { ptrAdapter->Release(); ptrAdapter = nullptr; }
			if( ptrOutput ) { ptrOutput->Release(); ptrOutput = nullptr; }
			if( ptrDisplayModes ) { delete[] ptrDisplayModes; ptrDisplayModes = nullptr; }

			ShutdownResources();
			return false;
		}

		Console::WriteLine( "[GOOD] DX11Renderer: Resource initialization complete!" );
		return true;
	}



	void DirectX11Graphics::BeginFrame()
	{
		assert( m_DeviceContext );
	}



	void DirectX11Graphics::EndFrame()
	{
		assert( m_SwapChain );

		// Present the frame
		m_SwapChain->Present( m_bVSync ? 1 : 0, 0 );
	}


	/*------------------------------------------------------------------------------------------
		DirectX11Graphics::ShutdownResources 
	------------------------------------------------------------------------------------------*/
	void DirectX11Graphics::ShutdownResources()
	{
		if( m_ScreenVertexList )	{ m_ScreenVertexList.Reset(); }
		if( m_ScreenIndexList )		{ m_ScreenIndexList.Reset(); }
		//if( m_CommonStates )		{ m_CommonStates.reset(); }
		if( m_BackBuffer )			{ m_BackBuffer.reset(); }
		//if( m_EffectFactory )		{ m_EffectFactory->ReleaseCache(); m_EffectFactory.reset(); }
		if( m_RasterizerState )		{ m_RasterizerState.Reset(); }
		if( m_DepthStencilState )	{ m_DepthStencilState.Reset(); }
		if( m_DepthDisabledState )	{ m_DepthDisabledState.Reset(); }
		if( m_DepthStencil )		{ m_DepthStencil.reset(); }
		if( m_BlendState )			{ m_BlendState.Reset(); }
		if( m_BlendDisabledState )	{ m_BlendDisabledState.Reset(); }
		if( m_RenderTarget )		{ m_RenderTarget.reset(); }
		if( m_DeviceContext )		{ m_DeviceContext.Reset(); }
		if( m_Device )				{ m_Device.Reset(); }
		if( m_SwapChain )			{ m_SwapChain->SetFullscreenState( false, nullptr ); m_SwapChain.Reset(); }
	}


	void DirectX11Graphics::SetCameraInfo( const ViewState& inView )
	{
		// Check if the camera has changed since last update
		if( !m_bRunning || m_CameraPosition.x != inView.Position.X || m_CameraPosition.y != inView.Position.Y || m_CameraPosition.z != inView.Position.Z ||
			m_CameraRotation.x != inView.Rotation.Pitch || m_CameraRotation.y != inView.Rotation.Yaw || m_CameraRotation.z != inView.Rotation.Roll || m_FOV != inView.FOV )
		{
			// Update cached camera potiion/rotation
			m_CameraPosition.x = inView.Position.X;
			m_CameraPosition.y = inView.Position.Y;
			m_CameraPosition.z = inView.Position.Z;

			m_CameraRotation.x = inView.Rotation.Pitch;
			m_CameraRotation.y = inView.Rotation.Yaw;
			m_CameraRotation.z = inView.Rotation.Roll;

			m_FOV = inView.FOV;

			// Re-calculate view matrix
			auto rot = DirectX::XMMatrixRotationRollPitchYaw(
				HYPERION_DEG_TO_RAD( m_CameraRotation.x ),
				HYPERION_DEG_TO_RAD( m_CameraRotation.y ),
				HYPERION_DEG_TO_RAD( m_CameraRotation.z )
			);

			auto up			= DirectX::XMVectorSet( 0.f, 1.f, 0.f, 1.f );
			auto dir		= DirectX::XMVectorSet( 0.f, 0.f, 1.f, 1.f );
			auto pos		= DirectX::XMLoadFloat3( &m_CameraPosition );

			dir		= DirectX::XMVector3TransformCoord( dir, rot );
			dir		= DirectX::XMVectorAdd( dir, pos );
			up		= DirectX::XMVector3TransformCoord( up, rot );

			m_ViewMatrix = DirectX::XMMatrixLookAtLH( pos, dir, up );

			// Build view frustum
			m_ViewFrustum.Construct( m_ViewMatrix, m_ProjectionMatrix, SCREEN_FAR );
		}	
	}


	bool DirectX11Graphics::CheckViewCull( const Transform3D& inTransform, const AABB& inBounds )
	{
		DirectX::XMFLOAT3 pos{ inTransform.Position.X, inTransform.Position.Y, inTransform.Position.Z };
		DirectX::XMFLOAT3 rot{ inTransform.Rotation.Pitch, inTransform.Rotation.Yaw, inTransform.Rotation.Roll };
		DirectX::XMFLOAT3 min{ inBounds.Min.X, inBounds.Min.Y, inBounds.Min.Z };
		DirectX::XMFLOAT3 max{ inBounds.Max.X, inBounds.Max.Y, inBounds.Max.Z };

		return m_ViewFrustum.CheckAABB( min, max, pos, rot );
	}


	/*------------------------------------------------------------------------------------------
		DirectX11Graphics::GenerateMatricies 
	------------------------------------------------------------------------------------------*/
	void DirectX11Graphics::GenerateMatricies( const ScreenResolution& inRes, float inFOV, float inNear, float inFar )
	{
		// Calculate values needed to generate matricies
		float aspectRatio	= (float) inRes.Width / (float) inRes.Height;
		auto up				= DirectX::XMVectorSet( 0.f, 1.f, 0.f, 1.f );
		auto lookAt			= DirectX::XMVectorSet( 0.f, 0.f, 1.f, 1.f );
		auto pos			= DirectX::XMVectorSet( 0.f, 0.f, 0.f, 1.f ); // TODO: In another project, I had this as -0.1f

		// Generate static matricies
		m_ProjectionMatrix	= DirectX::XMMatrixPerspectiveFovLH( inFOV, aspectRatio, inNear, inFar );
		m_WorldMatrix		= DirectX::XMMatrixIdentity();
		m_OrthoMatrix		= DirectX::XMMatrixOrthographicLH( (float)inRes.Width, (float)inRes.Height, inNear, inFar );
		m_ScreenMatrix		= DirectX::XMMatrixLookAtLH( pos, lookAt, up );
	}


	void DirectX11Graphics::GenerateScreenGeometry( uint32 inWidth, uint32 inHeight )
	{
		WindowVertex verts[ 6 ]{};

		float screenLeft	= -1.f * ( inWidth / 2.f );
		float screenRight	= screenLeft + inWidth;
		float screenTop		= ( inHeight / 2.f );
		float screenBottom	= screenTop - inHeight;

		verts[ 0 ].x = screenLeft;
		verts[ 0 ].y = screenTop;
		verts[ 0 ].z = 0.f;
		verts[ 0 ].u = 0.f;
		verts[ 0 ].v = 0.f;

		verts[ 1 ].x = screenRight;
		verts[ 1 ].y = screenBottom;
		verts[ 1 ].z = 0.f;
		verts[ 1 ].u = 1.f;
		verts[ 1 ].v = 1.f;

		verts[ 2 ].x = screenLeft;
		verts[ 2 ].y = screenBottom;
		verts[ 2 ].z = 0.f;
		verts[ 2 ].u = 0.f;
		verts[ 2 ].v = 1.f;

		verts[ 3 ].x = screenLeft;
		verts[ 3 ].y = screenTop;
		verts[ 3 ].z = 0.f;
		verts[ 3 ].u = 0.f;
		verts[ 3 ].v = 0.f;

		verts[ 4 ].x = screenRight;
		verts[ 4 ].y = screenTop;
		verts[ 4 ].z = 0.f;
		verts[ 4 ].u = 1.f;
		verts[ 4 ].v = 0.f;

		verts[ 5 ].x = screenRight;
		verts[ 5 ].y = screenBottom;
		verts[ 5 ].z = 0.f;
		verts[ 5 ].u = 1.f;
		verts[ 5 ].v = 1.f;

		uint32 indexList[ 6 ] = { 0, 1, 2, 3, 4, 5 };

		D3D11_BUFFER_DESC vertDesc;
		ZeroMemory( &vertDesc, sizeof( vertDesc ) );

		vertDesc.Usage					= D3D11_USAGE_DEFAULT;
		vertDesc.ByteWidth				= sizeof( WindowVertex ) * 6;
		vertDesc.BindFlags				= D3D11_BIND_VERTEX_BUFFER;
		vertDesc.CPUAccessFlags			= 0;
		vertDesc.MiscFlags				= 0;
		vertDesc.StructureByteStride	= 0;

		D3D11_SUBRESOURCE_DATA vertexData;
		ZeroMemory( &vertexData, sizeof( vertexData ) );

		vertexData.pSysMem				= (void*) verts;
		vertexData.SysMemPitch			= 0;
		vertexData.SysMemSlicePitch		= 0;

		if( FAILED( m_Device->CreateBuffer( &vertDesc, &vertexData, m_ScreenVertexList.GetAddressOf() ) ) || !m_ScreenVertexList )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create screen vertex list!" );
			return;
		}

		D3D11_BUFFER_DESC indexDesc;
		ZeroMemory( &indexDesc, sizeof( indexDesc ) );

		indexDesc.Usage					= D3D11_USAGE_DEFAULT;
		indexDesc.ByteWidth				= sizeof( uint32 ) * 6;
		indexDesc.BindFlags				= D3D11_BIND_INDEX_BUFFER;
		indexDesc.CPUAccessFlags		= 0;
		indexDesc.MiscFlags				= 0;
		indexDesc.StructureByteStride	= 0;

		D3D11_SUBRESOURCE_DATA indexData;
		ZeroMemory( &indexData, sizeof( indexData ) );

		indexData.pSysMem			= (void*) indexList;
		indexData.SysMemPitch		= 0;
		indexData.SysMemSlicePitch	= 0;

		if( FAILED( m_Device->CreateBuffer( &indexDesc, &indexData, m_ScreenIndexList.GetAddressOf() ) ) || !m_ScreenIndexList )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create screen index list!" );
			return;
		}
	}


	/*------------------------------------------------------------------------------------------
		DirectX11Graphics::EnableAlphaBlending 
	------------------------------------------------------------------------------------------*/
	void DirectX11Graphics::EnableAlphaBlending()
	{
		if( m_DeviceContext && m_BlendState )
		{
			float blendFactor[ 4 ] = { 0.f, 0.f, 0.f, 0.f };
			UINT sampleMask = 0xffffffff;

			m_DeviceContext->OMSetBlendState( m_BlendState.Get(), blendFactor, sampleMask );
		}
		else
		{
			Console::WriteLine( "[ERROR] DX11Renderer: Failed to enable alpha blending" );
		}
	}


	/*------------------------------------------------------------------------------------------
		DirectX11Graphics::DisableAlphaBlending 
	------------------------------------------------------------------------------------------*/
	void DirectX11Graphics::DisableAlphaBlending()
	{
		if( m_DeviceContext && m_BlendDisabledState )
		{
			float blendFactor[ 4 ] = { 0.f, 0.f, 0.f, 0.f };
			UINT sampleMask = 0xffffffff;

			m_DeviceContext->OMSetBlendState( m_BlendDisabledState.Get(), blendFactor, sampleMask );
		}
		else
		{
			Console::WriteLine( "[ERROR] DX11Renderer: Failed to disable alpha blending" );
		}
	}


	/*------------------------------------------------------------------------------------------
		DirectX11Graphics::IsAlphaBlendingEnabled 
	------------------------------------------------------------------------------------------*/
	bool DirectX11Graphics::IsAlphaBlendingEnabled()
	{
		if( m_DeviceContext && m_BlendState )
		{
			ID3D11BlendState* blendState = nullptr;
			float blendFactor[ 4 ];
			UINT sampleMask;

			m_DeviceContext->OMGetBlendState( &blendState, blendFactor, &sampleMask );
			return blendState == m_BlendState.Get();
		}
		else
		{
			Console::WriteLine( "[ERROR] DX11Renderer: Failed to check if alpha blending is enabled" );
			return false;
		}

		return false;
	}


	/*------------------------------------------------------------------------------------------
		DirectX11Graphics::EnableZBuffer 
	------------------------------------------------------------------------------------------*/
	void DirectX11Graphics::EnableZBuffer()
	{
		if( m_DeviceContext && m_DepthStencilState )
		{
			m_DeviceContext->OMSetDepthStencilState( m_DepthStencilState.Get(), 0 );
		}
		else
		{
			Console::WriteLine( "[ERROR] DX11Renderer: Failed to enable Z buffer" );
		}
	}


	/*------------------------------------------------------------------------------------------
		DirectX11Graphics::DisableZBuffer 
	------------------------------------------------------------------------------------------*/
	void DirectX11Graphics::DisableZBuffer()
	{
		if( m_DeviceContext && m_DepthDisabledState )
		{
			m_DeviceContext->OMSetDepthStencilState( m_DepthDisabledState.Get(), 0 );
		}
		else
		{
			Console::WriteLine( "[ERROR] DX11Renderer: Failed to dsiable Z buffer" );
		}
	}


	/*------------------------------------------------------------------------------------------
		DirectX11Graphics::IsZBufferEnabled 
	------------------------------------------------------------------------------------------*/
	bool DirectX11Graphics::IsZBufferEnabled()
	{
		if( m_DeviceContext && m_DepthStencilState )
		{
			UINT stencilNum;
			ID3D11DepthStencilState* state = nullptr;
			m_DeviceContext->OMGetDepthStencilState( &state, &stencilNum );
			return state == m_DepthStencilState.Get();
		}
		else
		{
			Console::WriteLine( "[ERROR] DX11Renderer: Failed to check if Z buffer is enabled" );
			return true;
		}

		return false;
	}


	std::shared_ptr< RRenderTarget > DirectX11Graphics::GetRenderTarget()
	{
		if( m_RenderTarget && m_RenderTarget->IsValid() )
		{
			return m_RenderTarget;
		}

		return nullptr;
	}


	std::shared_ptr< RTexture2D > DirectX11Graphics::GetBackBuffer()
	{
		if( m_BackBuffer && m_BackBuffer->IsValid() )
		{
			return m_BackBuffer;
		}

		return nullptr;
	}


	std::shared_ptr< RDepthStencil > DirectX11Graphics::GetDepthStencil()
	{
		if( m_DepthStencil && m_DepthStencil->IsValid() )
		{
			return m_DepthStencil;
		}

		return nullptr;
	}


	std::shared_ptr< RBuffer > DirectX11Graphics::CreateBuffer( const BufferParameters& inParams )
	{
		if( !m_Device )
		{
			Console::WriteLine( "[ERROR] DX11 API: Failed to create buffer.. device isnt valid" );
			return nullptr;
		}

		// Validate parameters
		if( inParams.Size > 0 && inParams.Data == nullptr )
		{
			Console::WriteLine( "[ERROR] DX11 API: Failed to create buffer, provided data was invalid!" );
			return nullptr;
		}

		// Create buffer object
		std::shared_ptr< DirectX11Buffer > newBuffer( new DirectX11Buffer( inParams.Type ) );

		D3D11_BUFFER_DESC Desc;
		ZeroMemory( &Desc, sizeof( Desc ) );

		switch( inParams.Type )
		{
		case BufferType::Vertex:
			Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			break;
		case BufferType::Index:
			Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			break;
		case BufferType::Constant:
			Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			break;
		case BufferType::ShaderResource:
			Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			break;
		case BufferType::StreamOutput:
			Desc.BindFlags = D3D11_BIND_STREAM_OUTPUT;
			break;
		}

		if( inParams.Dynamic )
		{
			Desc.Usage = D3D11_USAGE_DYNAMIC;
			Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			Desc.Usage = D3D11_USAGE_DEFAULT;
		}

		if( inParams.CanCPURead )
		{
			Desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
		}

		Desc.ByteWidth = inParams.Size;
		
		// Create subresource data
		D3D11_SUBRESOURCE_DATA Data;
		ZeroMemory( &Data, sizeof( Data ) );

		Data.pSysMem = inParams.Data;
		Data.SysMemPitch = 0;
		Data.SysMemSlicePitch = 0;

		// Create buffer
		if( FAILED( m_Device->CreateBuffer( &Desc, &Data, newBuffer->GetAddress() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11 API: Failed to create buffer!" );
			return nullptr;
		}

		if( !newBuffer->IsValid() )
		{
			Console::WriteLine( "[ERROR] DX11 API: Newly created buffer was not valid!" );
			return nullptr;
		}

		newBuffer->m_Size = inParams.Size;
		newBuffer->m_Count = inParams.Count;
		return newBuffer;
	}


	std::shared_ptr< RBuffer > DirectX11Graphics::CreateBuffer( BufferType inType /* = BufferType::Vertex */ )
	{
		return std::shared_ptr< DirectX11Buffer >( new DirectX11Buffer( inType ) );
	}


	// Texture Creation
	std::shared_ptr< RTexture1D > DirectX11Graphics::CreateTexture1D( const TextureParameters& inParams )
	{
		HYPERION_VERIFY( m_Device, "[DX11] Attempt to run graphics command, but the device is null!" );

		auto mipCount = (uint32)inParams.Data.size();
		if( mipCount > TEXTURE_MAX_LODS ) // TODO: Calculate max mip levels possible for this texture
		{
			Console::WriteLine( "[Warning] DX11: Failed to create Texture1D! Invalid number of mip levels specified! (", inParams.Data.size(), ")" );
			return nullptr;
		}
		else if( inParams.bAutogenMips && mipCount != 1 )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create Texture1D! Autogen mip levels selected, but invalid number of mips specified (", mipCount, ")" );
			return nullptr;
		}

		// Convert texture parameters into a texture description
		D3D11_TEXTURE1D_DESC Desc;
		ZeroMemory( &Desc, sizeof( Desc ) );

		Desc.Width		= inParams.Width;
		Desc.MipLevels	= inParams.bAutogenMips ? 0 : ( mipCount == 0 ? 1 : mipCount );
		Desc.ArraySize	= 1;
		Desc.MiscFlags	= 0;

		auto textureFormat	= TextureFormatToDXGIFormat( inParams.Format );
		Desc.Format			= textureFormat;
		Desc.BindFlags		= TranslateHyperionBindFlags( inParams.BindTargets );

		Desc.CPUAccessFlags = 0;
		if( inParams.bDynamic )
		{
			Desc.Usage				= D3D11_USAGE_DYNAMIC;
			Desc.CPUAccessFlags		|= D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			Desc.Usage = D3D11_USAGE_DEFAULT;
		}

		if( inParams.bCPURead )
		{
			Desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
		}

		// Create an array of subresource data structures for the mip data
		std::vector< D3D11_SUBRESOURCE_DATA > mipData;

		for( auto i = 0; i < inParams.Data.size(); i++ )
		{
			auto& res_data = mipData.emplace_back( D3D11_SUBRESOURCE_DATA() );

			res_data.pSysMem			= inParams.Data[ i ].Data;
			res_data.SysMemPitch		= 0;
			res_data.SysMemSlicePitch	= 0;
		}
		

		// Finally, lets create the texture
		std::shared_ptr< DirectX11Texture1D > Output( new DirectX11Texture1D() );
		if( FAILED( m_Device->CreateTexture1D( &Desc, mipData.empty() ? NULL : mipData.data(), Output->GetAddress() ) ) || Output->Get() == NULL )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 1D texture! API Call failed" );
			return nullptr;
		}

		// Create shader resource view if were setting the bind target to shader
		if( HYPERION_HAS_FLAG( inParams.BindTargets, TextureBindTarget::Shader ) )
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
			ZeroMemory( &viewDesc, sizeof( viewDesc ) );

			viewDesc.Format						= textureFormat;
			viewDesc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE1D;
			viewDesc.Texture1D.MipLevels		= (UINT) ( mipCount == 0 ? 1 : mipCount );
			viewDesc.Texture1D.MostDetailedMip	= 0;

			if( FAILED( m_Device->CreateShaderResourceView( Output->Get(), &viewDesc, Output->GetViewAddress() ) ) || Output->GetView() == NULL )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create 1D texture resource view! API call failed" );
				Output->Shutdown();

				return nullptr;
			}
		}

		return Output;
	}


	std::shared_ptr< RTexture2D > DirectX11Graphics::CreateTexture2D( const TextureParameters& inParams )
	{
		HYPERION_VERIFY( m_Device, "[DX11] Attempt to run graphics command, but the device is null!" );

		auto mipCount = (uint32)inParams.Data.size();
		if( mipCount > TEXTURE_MAX_LODS ) // TODO: Calculate max mip levels possible for this texture
		{
			Console::WriteLine( "[Warning] DX11: Failed to create Texture2D! Invalid number of mip levels specified! (", inParams.Data.size(), ")" );
			return nullptr;
		}
		else if( inParams.bAutogenMips && mipCount != 1 )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create Texture2D! Autogen mip levels selected, but invalid number of mips specified (", mipCount, ")" );
			return nullptr;
		}

		// Convert texture parameters into a texture description
		D3D11_TEXTURE2D_DESC Desc;
		ZeroMemory( &Desc, sizeof( Desc ) );

		Desc.Width			= inParams.Width;
		Desc.Height			= inParams.Height;
		Desc.MipLevels		= inParams.bAutogenMips ? 0 : ( mipCount == 0 ? 1 : mipCount );
		Desc.ArraySize		= 1;
		Desc.MiscFlags		= 0;

		auto textureFormat	= TextureFormatToDXGIFormat( inParams.Format );
		Desc.Format			= textureFormat;
		Desc.BindFlags		= TranslateHyperionBindFlags( inParams.BindTargets );

		// TODO: Implement multi-sampling in the generic api interface layer
		Desc.SampleDesc.Count		= 1;
		Desc.SampleDesc.Quality		= 0;

		Desc.CPUAccessFlags = 0;
		if( inParams.bDynamic )
		{
			Desc.Usage = D3D11_USAGE_DYNAMIC;
			Desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			Desc.Usage = D3D11_USAGE_DEFAULT;
		}

		if( inParams.bCPURead )
		{
			Desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
		}

		// Now that we have the format built, lets create the structure for the starting data (if any)
		std::vector< D3D11_SUBRESOURCE_DATA > mipData;

		for( uint32 i = 0; i < mipCount; i++ )
		{
			auto& data = mipData.emplace_back( D3D11_SUBRESOURCE_DATA() );

			data.pSysMem			= inParams.Data[ i ].Data;
			data.SysMemPitch		= inParams.Data[ i ].RowSize;
			data.SysMemSlicePitch	= 0;
		}

		// Finally, lets create the texture
		std::shared_ptr< DirectX11Texture2D > Output( new DirectX11Texture2D() );
		if( FAILED( m_Device->CreateTexture2D( &Desc, mipData.empty() ? NULL : mipData.data(), Output->GetAddress() ) ) || Output->Get() == NULL )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 2D texture! API Call failed" );
			return nullptr;
		}

		// Create shader resource view if were setting the bind target to shader
		if( HYPERION_HAS_FLAG( inParams.BindTargets, TextureBindTarget::Shader ) )
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
			ZeroMemory( &viewDesc, sizeof( viewDesc ) );

			viewDesc.Format						= textureFormat;
			viewDesc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipLevels		= (UINT) ( mipCount == 0 ? 1 : mipCount );
			viewDesc.Texture2D.MostDetailedMip	= 0;

			if( FAILED( m_Device->CreateShaderResourceView( Output->Get(), &viewDesc, Output->GetViewAddress() ) ) || Output->GetView() == NULL )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create 2D texture resource view! API call failed" );
				Output->Shutdown();

				return nullptr;
			}
		}

		return Output;
	}


	std::shared_ptr< RTexture3D > DirectX11Graphics::CreateTexture3D( const TextureParameters& inParams )
	{
		HYPERION_VERIFY( m_Device, "[DX11] Attempt to run graphics command, but the device is null!" );

		auto mipCount = (uint32)inParams.Data.size();
		if( mipCount > TEXTURE_MAX_LODS ) // TODO: Calculate max mip levels possible for this texture
		{
			Console::WriteLine( "[Warning] DX11: Failed to create Texture3D! Invalid number of mip levels specified! (", inParams.Data.size(), ")" );
			return nullptr;
		}
		else if( inParams.bAutogenMips && mipCount != 1 )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create Texture3D! Autogen mip levels selected, but invalid number of mips specified (", mipCount, ")" );
			return nullptr;
		}

		// Convert texture parameters into a texture description
		D3D11_TEXTURE3D_DESC Desc;
		ZeroMemory( &Desc, sizeof( Desc ) );

		Desc.Width		= inParams.Width;
		Desc.Height		= inParams.Height;
		Desc.Depth		= inParams.Depth;

		Desc.MipLevels = inParams.bAutogenMips ? 0 : ( mipCount == 0 ? 1 : mipCount );
		Desc.MiscFlags = 0;

		auto textureFormat	= TextureFormatToDXGIFormat( inParams.Format );
		Desc.Format			= textureFormat;
		Desc.BindFlags		= TranslateHyperionBindFlags( inParams.BindTargets );

		Desc.CPUAccessFlags = 0;
		if( inParams.bDynamic )
		{
			Desc.Usage = D3D11_USAGE_DYNAMIC;
			Desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			Desc.Usage = D3D11_USAGE_DEFAULT;
		}

		if( inParams.bCPURead )
		{
			Desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
		}

		// Now that we have the format built, lets create the structure for the starting data (if any)
		std::vector< D3D11_SUBRESOURCE_DATA > mipData;

		for( uint32 i = 0; i < mipCount; i++ )
		{
			auto& data = mipData.emplace_back( D3D11_SUBRESOURCE_DATA() );

			data.pSysMem			= inParams.Data[ i ].Data;
			data.SysMemPitch		= inParams.Data[ i ].RowSize;
			data.SysMemSlicePitch	= inParams.Data[ i ].LayerSize;
		}

		// Finally, lets create the texture
		std::shared_ptr< DirectX11Texture3D > Output( new DirectX11Texture3D() );
		if( FAILED( m_Device->CreateTexture3D( &Desc, mipData.empty() ? NULL : mipData.data(), Output->GetAddress() ) ) || Output->Get() == NULL )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 3D texture! API Call failed" );
			return nullptr;
		}

		// Create shader resource view if were setting the bind target to shader
		if( HYPERION_HAS_FLAG( inParams.BindTargets, TextureBindTarget::Shader ) )
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
			ZeroMemory( &viewDesc, sizeof( viewDesc ) );

			viewDesc.Format						= textureFormat;
			viewDesc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE3D;
			viewDesc.Texture3D.MipLevels		= (UINT) ( mipCount == 0 ? 1 : mipCount );
			viewDesc.Texture3D.MostDetailedMip	= 0;

			if( FAILED( m_Device->CreateShaderResourceView( Output->Get(), &viewDesc, Output->GetViewAddress() ) ) || Output->GetView() == NULL )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create 3D texture resource view! API call failed" );
				Output->Shutdown();

				return nullptr;
			}
		}

		return Output;
	}


	std::shared_ptr< RTexture1D > DirectX11Graphics::CreateTexture1D()
	{
		//HYPERION_VERIFY( m_Device, "[DX11] Attempt to run graphics command, but the device is null!" );
		// Create and return an empty texture pointer
		return std::shared_ptr< DirectX11Texture1D >( new DirectX11Texture1D() );
	}


	std::shared_ptr< RTexture2D > DirectX11Graphics::CreateTexture2D()
	{
		//HYPERION_VERIFY( m_Device, "[DX11] Attempt to run graphics command, but the device is null!" );
		// Create and return an empty texture pointer
		return std::shared_ptr< DirectX11Texture2D >( new DirectX11Texture2D() );
	}


	std::shared_ptr< RTexture3D > DirectX11Graphics::CreateTexture3D()
	{
		//HYPERION_VERIFY( m_Device, "[DX11] Attempt to run graphics command, but the device is null!" );
		// Create and return an empty texture pointer
		return std::shared_ptr< DirectX11Texture3D >( new DirectX11Texture3D() );
	}


	// Texture Copying
	bool DirectX11Graphics::CopyTexture1D( std::shared_ptr< RTexture1D >& inSource, std::shared_ptr< RTexture1D >& inDest )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Attempt to run graphics command, but the device is null!" );

		// Verify both textures are valid and allocated
		if( !inSource || !inDest || !inSource->IsValid() || !inDest->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to copy 1D Texture! Either the source or destination were invalid/null" );
			return false;
		} 

		// Cast the textures to our API's type
		DirectX11Texture1D* sourcePtr	= dynamic_cast< DirectX11Texture1D* >( inSource.get() );
		DirectX11Texture1D* destPtr		= dynamic_cast< DirectX11Texture1D* >( inDest.get() );

		HYPERION_VERIFY( sourcePtr != nullptr && destPtr != nullptr, "[DX11] Texture instances were not this API's texture type?" );

		// Send the copy command to the GPU
		m_DeviceContext->CopyResource( destPtr->Get(), sourcePtr->Get() );
		return true;
	}


	bool DirectX11Graphics::CopyTexture2D( std::shared_ptr< RTexture2D >& inSource, std::shared_ptr< RTexture2D >& inDest )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Attempt to run graphics command, but the device is null!" );

		// Verify both textures are valid and allocated
		if( !inSource || !inDest || !inSource->IsValid() || !inDest->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to copy 2D Texture! Either the source or destination were invalid/null" );
			return false;
		}

		// Cast the textures to our API's type
		DirectX11Texture2D* sourcePtr	= dynamic_cast<DirectX11Texture2D*>( inSource.get() );
		DirectX11Texture2D* destPtr		= dynamic_cast<DirectX11Texture2D*>( inDest.get() );

		HYPERION_VERIFY( sourcePtr != nullptr && destPtr != nullptr, "[DX11] Texture instances were not this API's texture type?" );

		// Send the copy command to the GPU
		m_DeviceContext->CopyResource( destPtr->Get(), sourcePtr->Get() );
		return true;
	}


	bool DirectX11Graphics::CopyTexture3D( std::shared_ptr< RTexture3D >& inSource, std::shared_ptr< RTexture3D >& inDest )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Attempt to run graphics command, but the device is null!" );

		// Verify both textures are valid and allocated
		if( !inSource || !inDest || !inSource->IsValid() || !inDest->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to copy 3D Texture! Either the source or destination were invalid/null" );
			return false;
		}

		// Cast the textures to our API's type
		DirectX11Texture3D* sourcePtr	= dynamic_cast<DirectX11Texture3D*>( inSource.get() );
		DirectX11Texture3D* destPtr		= dynamic_cast<DirectX11Texture3D*>( inDest.get() );

		HYPERION_VERIFY( sourcePtr != nullptr && destPtr != nullptr, "[DX11] Texture instances were not this API's texture type?" );

		// Send the copy command to the GPU
		m_DeviceContext->CopyResource( destPtr->Get(), sourcePtr->Get() );
		return true;
	}


	bool DirectX11Graphics::CopyTexture1DRegion( std::shared_ptr< RTexture1D >& inSource, std::shared_ptr< RTexture1D >& inDest,
							  uint32 sourceX, uint32 inWidth, uint32 destX, uint8 sourceMip, uint8 targetMip )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Attempt to run graphics command, but the device is null!" );

		// Validate the texture pointers
		if( !inSource || !inDest || !inSource->IsValid() || !inDest->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to copy 1D texture! Either the source or destination are invalid/null" );
			return false;
		}

		// Ensure the copy is within valid bounds
		uint32 maxSourceX	= sourceX + inWidth;
		uint32 maxDestX		= destX + inWidth;

		if( maxSourceX > inSource->GetWidth() || maxDestX > inDest->GetWidth() || sourceMip > inSource->GetMipCount() || targetMip > inDest->GetMipCount() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to copy 1D texture! The target region is out of bounds for either the source or destination texture" );
			return false;
		}

		// Get our API's texture pointers
		DirectX11Texture1D* sourcePtr	= dynamic_cast< DirectX11Texture1D* >( inSource.get() );
		DirectX11Texture1D* destPtr		= dynamic_cast< DirectX11Texture1D* >( inDest.get() );

		HYPERION_VERIFY( sourcePtr != nullptr && destPtr != nullptr, "[DX11] Textures instances were not this API's texture type?" );

		// Upload copy command to the GPU
		D3D11_BOX bounds;
		bounds.left = sourceX;
		bounds.right = maxSourceX;
		bounds.top = 0;
		bounds.bottom = 1;
		bounds.front = 0;
		bounds.back = 1;

		m_DeviceContext->CopySubresourceRegion( destPtr->Get(), targetMip, destX, 0, 0,
												sourcePtr->Get(), sourceMip, &bounds );
		return true;
	}


	bool DirectX11Graphics::CopyTexture2DRegion( std::shared_ptr< RTexture2D >& inSource, std::shared_ptr< RTexture2D >& inDest, uint32 sourceX, uint32 sourceY,
							  uint32 inWidth, uint32 inHeight, uint32 destX, uint32 destY, uint8 sourceMip, uint8 targetMip )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Attempt to run graphics command, but the device is null!" );

		// Validate the texture pointers
		if( !inSource || !inDest || !inSource->IsValid() || !inDest->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to copy 2D texture! Either the source or destination are invalid/null" );
			return false;
		}

		// Ensure the copy is within valid bounds
		uint32 maxSourceX	= sourceX + inWidth;
		uint32 maxDestX		= destX + inWidth;
		uint32 maxSourceY	= sourceY + inHeight;
		uint32 maxDestY		= destY + inHeight;

		if( maxSourceX > inSource->GetWidth() || maxDestX > inDest->GetWidth() || sourceMip > inSource->GetMipCount() || targetMip > inDest->GetMipCount() ||
			maxSourceY > inSource->GetHeight() || maxDestY > inDest->GetHeight() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to copy 2D texture! The target region is out of bounds for either the source or destination texture" );
			return false;
		}

		// Get our API's texture pointers
		DirectX11Texture2D* sourcePtr	= dynamic_cast< DirectX11Texture2D* >( inSource.get() );
		DirectX11Texture2D* destPtr		= dynamic_cast< DirectX11Texture2D* >( inDest.get() );

		HYPERION_VERIFY( sourcePtr != nullptr && destPtr != nullptr, "[DX11] Textures instances were not this API's texture type?" );

		// Upload copy command to the GPU
		D3D11_BOX bounds;
		bounds.left = sourceX;
		bounds.right = maxSourceX;
		bounds.top = sourceY;
		bounds.bottom = maxSourceY;
		bounds.front = 0;
		bounds.back = 1;

		m_DeviceContext->CopySubresourceRegion( destPtr->Get(), targetMip, destX, destY, 0,
												sourcePtr->Get(), sourceMip, &bounds );
		return true;
	}


	bool DirectX11Graphics::CopyTexture3DRegion( std::shared_ptr< RTexture3D >& inSource, std::shared_ptr< RTexture3D >& inDest, uint32 sourceX, uint32 sourceY, uint32 sourceZ,
							  uint32 inWidth, uint32 inHeight, uint32 inDepth, uint32 destX, uint32 destY, uint32 destZ, uint8 sourceMip, uint8 targetMip )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Attempt to run graphics command, but the device is null!" );

		// Validate the texture pointers
		if( !inSource || !inDest || !inSource->IsValid() || !inDest->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to copy 3D texture! Either the source or destination are invalid/null" );
			return false;
		}

		// Ensure the copy is within valid bounds
		uint32 maxSourceX	= sourceX + inWidth;
		uint32 maxDestX		= destX + inWidth;
		uint32 maxSourceY	= sourceY + inHeight;
		uint32 maxDestY		= destY + inHeight;
		uint32 maxSourceZ	= sourceZ + inDepth;
		uint32 maxDestZ		= destZ + inDepth;

		if( maxSourceX > inSource->GetWidth() || maxDestX > inDest->GetWidth() || sourceMip > inSource->GetMipCount() || targetMip > inDest->GetMipCount() ||
			maxSourceY > inSource->GetHeight() || maxDestY > inDest->GetHeight() || maxSourceZ > inSource->GetDepth() || maxDestZ > inDest->GetDepth() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to copy 3D texture! The target region is out of bounds for either the source or destination texture" );
			return false;
		}

		// Get our API's texture pointers
		DirectX11Texture3D* sourcePtr	= dynamic_cast< DirectX11Texture3D* >( inSource.get() );
		DirectX11Texture3D* destPtr		= dynamic_cast< DirectX11Texture3D* >( inDest.get() );

		HYPERION_VERIFY( sourcePtr != nullptr && destPtr != nullptr, "[DX11] Textures instances were not this API's texture type?" );

		// Upload copy command to the GPU
		D3D11_BOX bounds;
		bounds.left = sourceX;
		bounds.right = maxSourceX;
		bounds.top = sourceY;
		bounds.bottom = maxSourceY;
		bounds.front = sourceZ;
		bounds.back = maxSourceZ;

		m_DeviceContext->CopySubresourceRegion( destPtr->Get(), targetMip, destX, destY, destZ,
												sourcePtr->Get(), sourceMip, &bounds );
		return true;
	}


	bool DirectX11Graphics::CopyTexture1DMip( std::shared_ptr< RTexture1D >& inSource, std::shared_ptr< RTexture1D >& inDest, uint8 sourceMip, uint8 destMip )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Attempt to run graphics command, but the device is null!" );

		// Validate the texture pointers
		if( !inSource || !inDest || !inSource->IsValid() || !inDest->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to copy a 1D texture mip level! The source or destination textures were invalid" );
			return false;
		}

		// Validate the target mip levels are valid
		if( sourceMip > inSource->GetMipCount() || destMip > inDest->GetMipCount() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to copy a 1D texture mip level! The destination or target mip level was out of bounds!" );
			return false;
		}

		// TODO: Validate the mip sizes are the same

		// Get the API's texture pointers
		DirectX11Texture1D* sourcePtr	= dynamic_cast< DirectX11Texture1D* >( inSource.get() );
		DirectX11Texture1D* destPtr		= dynamic_cast< DirectX11Texture1D* >( inDest.get() );

		HYPERION_VERIFY( sourcePtr != nullptr && destPtr != nullptr, "[DX11] Texture instances were not this API's type?" );

		// Send the copy command to the GPU
		m_DeviceContext->CopySubresourceRegion( destPtr->Get(), destMip, 0, 0, 0,
												sourcePtr->Get(), sourceMip, NULL );
		return true;
	}


	bool DirectX11Graphics::CopyTexture2DMip( std::shared_ptr< RTexture2D >& inSource, std::shared_ptr< RTexture2D >& inDest, uint8 sourceMip, uint8 destMip )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Attempt to run graphics command, but the device is null!" );

		// Validate the target mip levels are valid
		if( sourceMip > inSource->GetMipCount() || destMip > inDest->GetMipCount() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to copy a 2D texture mip level! The destination or target mip level was out of bounds!" );
			return false;
		}

		// TODO: Validate the mip sizes are the same

		// Get the API's texture pointers
		DirectX11Texture2D* sourcePtr	= dynamic_cast< DirectX11Texture2D* >( inSource.get() );
		DirectX11Texture2D* destPtr		= dynamic_cast< DirectX11Texture2D* >( inDest.get() );

		HYPERION_VERIFY( sourcePtr != nullptr && destPtr != nullptr, "[DX11] Texture instances were not this API's type?" );

		// Send the copy command to the GPU
		m_DeviceContext->CopySubresourceRegion( destPtr->Get(), destMip, 0, 0, 0,
												sourcePtr->Get(), sourceMip, NULL );
		return true;
	}


	bool DirectX11Graphics::CopyTexture3DMip( std::shared_ptr< RTexture3D >& inSource, std::shared_ptr< RTexture3D >& inDest, uint8 sourceMip, uint8 destMip )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Attempt to run graphics command, but the device is null!" );

		// Validate the target mip levels are valid
		if( sourceMip > inSource->GetMipCount() || destMip > inDest->GetMipCount() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to copy a 3D texture mip level! The destination or target mip level was out of bounds!" );
			return false;
		}

		// TODO: Validate the mip sizes are the same

		// Get the API's texture pointers
		DirectX11Texture3D* sourcePtr	= dynamic_cast< DirectX11Texture3D* >( inSource.get() );
		DirectX11Texture3D* destPtr		= dynamic_cast< DirectX11Texture3D* >( inDest.get() );

		HYPERION_VERIFY( sourcePtr != nullptr && destPtr != nullptr, "[DX11] Texture instances were not this API's type?" );

		// Send the copy command to the GPU
		m_DeviceContext->CopySubresourceRegion( destPtr->Get(), destMip, 0, 0, 0,
												sourcePtr->Get(), sourceMip, NULL );
		return true;
	}


	std::shared_ptr< RRenderTarget > DirectX11Graphics::CreateRenderTarget( const std::shared_ptr< RTexture2D >& inTarget )
	{
		HYPERION_VERIFY( m_Device, "[DX11] Device was null!" );

		auto output = std::shared_ptr< DirectX11RenderTarget >( new DirectX11RenderTarget( inTarget, m_Device ) );
		if( !output->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create render target!" );
			return nullptr;
		}

		return output;
	}


	void DirectX11Graphics::ClearRenderTarget( const std::shared_ptr< RRenderTarget >& inTarget, const Color4F& inColor )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Context was null" );

		if( !inTarget || !inTarget->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to clear render target!" );
			return;
		}

		auto* targetPtr = dynamic_cast< DirectX11RenderTarget* >( inTarget.get() );
		HYPERION_VERIFY( targetPtr, "[DX11] Failed to cast render target to correct type" );

		FLOAT color[ 4 ] = { inColor.r, inColor.g, inColor.b, inColor.a };

		m_DeviceContext->ClearRenderTargetView( targetPtr->Get(), color );
	}


	std::shared_ptr< RDepthStencil > DirectX11Graphics::CreateDepthStencil( uint32 inWidth, uint32 inHeight )
	{
		HYPERION_VERIFY( m_Device, "[DX11] Device was null!" );

		auto output = std::shared_ptr< RDepthStencil >( new DirectX11DepthStencil( m_Device.Get(), inWidth, inHeight ) );
		if( !output->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create depth stencil" );
			return nullptr;
		}

		return output;
	}


	bool DirectX11Graphics::ResizeDepthStencil( const std::shared_ptr< RDepthStencil >& inStencil, uint32 inWidth, uint32 inHeight )
	{
		HYPERION_VERIFY( m_Device, "[DX11] Device was null!" );

		if( inStencil )
		{
			auto casted = std::dynamic_pointer_cast<DirectX11DepthStencil>( inStencil );
			return casted ? casted->Resize( m_Device.Get(), inWidth, inHeight ) : false;
		}

		return false;
	}


	void DirectX11Graphics::ClearDepthStencil( const std::shared_ptr< RDepthStencil >& inStencil, const Color4F& inColor )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Context was null" );

		if( !inStencil || !inStencil->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to clear depth stencil" );
			return;
		}

		auto* viewPtr = dynamic_cast< DirectX11DepthStencil* >( inStencil.get() );
		HYPERION_VERIFY( viewPtr, "[DX11] Failed to cast depth stencil view to api type" );

		m_DeviceContext->ClearDepthStencilView( viewPtr->GetStencilView(), D3D11_CLEAR_DEPTH, 1.f, 0 );
	}


	std::shared_ptr<RGBufferShader> DirectX11Graphics::CreateGBufferShader( const String& inPixelShader, const String& inVertexShader )
	{
		// Validate parameters
		if( inPixelShader.IsWhitespaceOrEmpty() || inVertexShader.IsWhitespaceOrEmpty() )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create GBuffer shader! The target path(s) were invalid" );
			return nullptr;
		}

		// Create shader instance
		auto newShader = std::make_shared< DirectX11GBufferShader >( inPixelShader, inVertexShader );
		if( !newShader->Initialize( m_Device.Get(), m_DeviceContext.Get() ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create GBuffer shader! Couldnt initialize shader" );
			return nullptr;
		}
		
		return newShader;
	}


	std::shared_ptr<RLightingShader> DirectX11Graphics::CreateLightingShader( const String& inPixelShader, const String& inVertexShader )
	{
		// Validate parameters
		if( inPixelShader.IsWhitespaceOrEmpty() || inVertexShader.IsWhitespaceOrEmpty() )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create GBuffer shader! The target path(s) were invalid" );
			return nullptr;
		}

		// Create shader instance
		auto newShader = std::make_shared< DirectX11LightingShader >( inPixelShader, inVertexShader );
		if( !newShader->Initialize( m_Device.Get(), m_DeviceContext.Get() ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create Lighting shader! Couldnt initialize shader" );
			return nullptr;
		}

		return newShader;
	}


	std::shared_ptr<RForwardShader> DirectX11Graphics::CreateForwardShader( const String& inPixelShader, const String& inVertexShader )
	{
		// Validate parameters
		if( inPixelShader.IsWhitespaceOrEmpty() || inVertexShader.IsWhitespaceOrEmpty() )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create Forward shader! The target path(s) were invalid" );
			return nullptr;
		}

		auto newShader = std::make_shared< DirectX11ForwardShader >( inPixelShader, inVertexShader );
		if( !newShader->Initialize( m_Device.Get(), m_DeviceContext.Get() ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create Forward shader! Couldnt initialize shader" );
			return nullptr;
		}

		return newShader;
	}


	std::shared_ptr<RComputeShader> DirectX11Graphics::CreateComputeShader( const String& inShader )
	{
		// Validate parameters
		if( inShader.IsWhitespaceOrEmpty() )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create compute shader! The target path were invalid" );
			return nullptr;
		}

		return std::shared_ptr<RComputeShader>();
	}


	/*
	*	Rendering
	*/
	void DirectX11Graphics::SetShader( const std::shared_ptr< RShader >& inShader )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Device Context was null!" );

		// Validate the shader
		if( !inShader || !inShader->IsValid() )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to set shader pipeline! The parameter was null" );
			return;
		}

		ID3D11VertexShader* vertexShader	= nullptr;
		ID3D11PixelShader* pixelShader		= nullptr;
		ID3D11InputLayout* inputLayout		= nullptr;

		switch( inShader->GetType() )
		{
		case ShaderType::Forward:
		{
			auto shaderPtr = dynamic_cast<DirectX11ForwardShader*>( inShader.get() );
			HYPERION_VERIFY( shaderPtr != nullptr, "[DX11] Failed to cast shader to correct type" );

			vertexShader	= shaderPtr->GetVertexShader();
			pixelShader		= shaderPtr->GetPixelShader();
			inputLayout		= shaderPtr->GetInputLayout();

			break;
		}
		case ShaderType::GBuffer:
		{
			auto gshaderPtr = dynamic_cast<DirectX11GBufferShader*>( inShader.get() );
			HYPERION_VERIFY( gshaderPtr != nullptr, "[DX11] Failed to cast shader to correct type" );

			vertexShader	= gshaderPtr->GetVertexShader();
			pixelShader		= gshaderPtr->GetPixelShader();
			inputLayout		= gshaderPtr->GetInputLayout();

			break;
		}
		case ShaderType::Lighting:
		{
			auto lshaderPtr = dynamic_cast<DirectX11LightingShader*>( inShader.get() );
			HYPERION_VERIFY( lshaderPtr != nullptr, "[DX11] Failed to cast shader to correct type" );

			vertexShader	= lshaderPtr->GetVertexShader();
			pixelShader		= lshaderPtr->GetPixelShader();
			inputLayout		= lshaderPtr->GetInputLayout();

			break;
		}
		default:

			HYPERION_VERIFY( true, "[DX11] Attempt to render primitive with an invalid shader type" );
			break;
		}

		if( !vertexShader || !pixelShader || !inputLayout )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to set shader pipeline! The shader was invalid" );
			return;
		}

		// Set the shaders
		m_DeviceContext->IASetInputLayout( inputLayout );
		m_DeviceContext->VSSetShader( vertexShader, NULL, 0 );
		m_DeviceContext->PSSetShader( pixelShader, NULL, 0 );
	}


	void DirectX11Graphics::SetRenderOutputToScreen()
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Device context was null" );
		HYPERION_VERIFY( m_RenderTarget && m_RenderTarget->IsValid(), "[DX11] Back buffer render target was null/invalid" );

		ID3D11RenderTargetView* views[] = { m_RenderTarget->Get() };
		m_DeviceContext->OMSetRenderTargets( 1, views, m_DepthStencil->GetStencilView() );

		D3D11_VIEWPORT viewport{};

		viewport.Width		= (FLOAT)m_Resolution.Width;
		viewport.Height		= (FLOAT)m_Resolution.Height;
		viewport.MinDepth	= 0.f;
		viewport.MaxDepth	= 1.f;
		viewport.TopLeftX	= 0.f;
		viewport.TopLeftY	= 1.f;
		
		m_DeviceContext->RSSetViewports( 1, &viewport );
	}


	void DirectX11Graphics::SetRenderOutputToTarget( const std::shared_ptr< RRenderTarget >& inTarget, const std::shared_ptr< RDepthStencil >& inStencil )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Device context was null" );

		// Ensure the parameters are valid
		if( !inTarget || !inStencil || !inTarget->IsValid() || !inStencil->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to set render output to target, the render target or depth stencil was invalid" );
			return;
		}

		auto targetTexture = inTarget->GetTargetTexture();
		HYPERION_VERIFY( targetTexture, "[DX11] Failed to get target texture from render target even though IsValid returned true" );

		auto* targetPtr		= dynamic_cast< DirectX11RenderTarget* >( inTarget.get() );
		auto* stencilPtr	= dynamic_cast< DirectX11DepthStencil* >( inStencil.get() );
		HYPERION_VERIFY( targetPtr && stencilPtr, "[DX11] Failed to cast render target and depth stencil to api type" );

		ID3D11RenderTargetView* views[] = { targetPtr->Get() };
		m_DeviceContext->OMSetRenderTargets( 1, views, stencilPtr->GetStencilView() );

		D3D11_VIEWPORT viewport{};

		viewport.Width		= (FLOAT)targetTexture->GetWidth();
		viewport.Height		= (FLOAT)targetTexture->GetHeight();
		viewport.MinDepth	= 0.f;
		viewport.MaxDepth	= 1.f;
		viewport.TopLeftX	= 0.f;
		viewport.TopLeftY	= 1.f;

		HYPERION_VERIFY( viewport.Width > 0 && viewport.Height > 0, "[DX11] Render output to texture with invalid width/height" );

		m_DeviceContext->RSSetViewports( 1, &viewport );
	}


	void DirectX11Graphics::SetRenderOutputToGBuffer( const std::shared_ptr< GBuffer >& inBuffer )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Device context was null" );

		if( !inBuffer || !inBuffer->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to set render output to the gbuffer, the gbuffer was invalid" );
			return;
		}

		// Set the OM output to the textures in the g-buffer
		auto* diffPtr		= dynamic_cast< DirectX11RenderTarget* >( inBuffer->GetDiffuseRoughnessTarget().get() );
		auto* normPtr		= dynamic_cast< DirectX11RenderTarget* >( inBuffer->GetNormalDepthTarget().get() );
		auto* specPtr		= dynamic_cast< DirectX11RenderTarget* >( inBuffer->GetSpecularTarget().get() );
		auto* depthStencil	= dynamic_cast< DirectX11DepthStencil* >( inBuffer->GetDepthStencil().get() );

		// These shouldnt fail, since IsValid() returned true
		HYPERION_VERIFY( diffPtr && normPtr && specPtr, "[DX11] Render targets couldnt be casted to the api type?" );
		HYPERION_VERIFY( depthStencil, "[DX11] Depth stencil couldnt be casted to api type?" );

		

		ID3D11RenderTargetView* renderTargets[] = { diffPtr->Get(), normPtr->Get(), specPtr->Get() };
		m_DeviceContext->OMSetRenderTargets( 3, renderTargets, depthStencil->GetStencilView() );

		D3D11_VIEWPORT viewport{};

		viewport.Width		= (FLOAT)inBuffer->GetWidth();
		viewport.Height		= (FLOAT)inBuffer->GetHeight();
		viewport.MinDepth	= 0.f;
		viewport.MaxDepth	= 1.f;
		viewport.TopLeftX	= 0.f;
		viewport.TopLeftY	= 1.f;

		HYPERION_VERIFY( viewport.Width > 0 && viewport.Height > 0, "[DX11] GBuffer has invalid dimensions?" );

		m_DeviceContext->RSSetViewports( 1, &viewport );
	}


	void DirectX11Graphics::RenderGeometry( const std::shared_ptr< RBuffer >& inVertexBuffer, const std::shared_ptr< RBuffer >& inIndexBuffer, uint32 inIndexCount )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Device context was null" );

		if( inIndexCount < 3 ) { return; }

		auto* vertexBuffer	= inVertexBuffer ?	dynamic_cast<DirectX11Buffer*>( inVertexBuffer.get() ) :	nullptr;
		auto* indexBuffer	= inIndexBuffer ?	dynamic_cast<DirectX11Buffer*>( inIndexBuffer.get() ) :		nullptr;

		// Validate the buffers
		if( !vertexBuffer || !indexBuffer || !vertexBuffer->IsValid() || !indexBuffer->IsValid() ) { return; }
		if( vertexBuffer->GetSize() == 0 || indexBuffer->GetSize() < 3 ) { return; }

		ID3D11Buffer* vertexBuffers[]	= { vertexBuffer->GetBuffer() };
		UINT vertexStrides[]			= { sizeof( Vertex3D ) };
		UINT vertexOffsets[]			= { 0 };

		m_DeviceContext->IASetVertexBuffers( 0, 1, vertexBuffers, vertexStrides, vertexOffsets );
		m_DeviceContext->IASetIndexBuffer( indexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0 );
		m_DeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		m_DeviceContext->DrawIndexed( inIndexCount, 0, 0 );
	}


	void DirectX11Graphics::RenderScreenGeometry()
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Device context was null!" );
		HYPERION_VERIFY( m_ScreenVertexList && m_ScreenIndexList, "[DX11] Screen geometry was null!" );

		ID3D11Buffer* vertexBuffers[]	= { m_ScreenVertexList.Get() };
		UINT vertexStrides[]			= { 0 };
		UINT vertexOffsets[]			= { 0 };

		m_DeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		m_DeviceContext->IASetVertexBuffers( 0, 1, vertexBuffers, vertexStrides, vertexOffsets );
		m_DeviceContext->IASetIndexBuffer( m_ScreenIndexList.Get(), DXGI_FORMAT_R32_UINT, 0 );
		m_DeviceContext->DrawIndexed( 6, 0, 0 );
	}


	void DirectX11Graphics::GetWorldMatrix( const Transform3D& inObj, Matrix& outMatrix )
	{
		// We need to calculate the relative position and rotation of the object to the camera
		auto relativePosition = DirectX::XMFLOAT3( inObj.Position.X - m_CameraPosition.x, inObj.Position.Y - m_CameraPosition.y, inObj.Position.Z - m_CameraPosition.z );
		auto relativeRotation = DirectX::XMFLOAT3( inObj.Rotation.Pitch - m_CameraRotation.x, inObj.Rotation.Yaw - m_CameraRotation.y, inObj.Rotation.Roll - m_CameraRotation.z );

		auto out	= m_WorldMatrix;
		out			*= DirectX::XMMatrixRotationRollPitchYaw( HYPERION_DEG_TO_RAD( relativeRotation.x ), HYPERION_DEG_TO_RAD( relativeRotation.y ), HYPERION_DEG_TO_RAD( relativeRotation.z ) );
		out			*= DirectX::XMMatrixTranslation( relativePosition.x, relativePosition.y, relativePosition.z );

		outMatrix.AssignData( out.r[ 0 ].m128_f32 );
	}


	void DirectX11Graphics::GetWorldMatrix( Matrix& outMatrix )
	{
		// TODO: Need a better/cleaner solution
		// It would be ideal, if the shaders were able to grab the matricies they needed from us
		// but, thats not very clean either, we will come back to this..
		outMatrix = m_WorldMatrix.r[ 0 ].m128_f32;
	}


	void DirectX11Graphics::GetViewMatrix( Matrix& outMatrix )
	{
		// TODO: This is dirty
		outMatrix.AssignData( m_ViewMatrix.r[ 0 ].m128_f32 );
	}


	void DirectX11Graphics::GetProjectionMatrix( Matrix& outMatrix )
	{
		// TODO
		outMatrix.AssignData( m_ProjectionMatrix.r[ 0 ].m128_f32 );
	}


	void DirectX11Graphics::GetOrthoMatrix( Matrix& outMatrix )
	{
		// TODO
		outMatrix.AssignData( m_OrthoMatrix.r[ 0 ].m128_f32 );
	}


	void DirectX11Graphics::GetScreenMatrix( Matrix& outMatrix )
	{
		// TODO
		outMatrix.AssignData( m_ScreenMatrix.r[ 0 ].m128_f32 );
	}

}