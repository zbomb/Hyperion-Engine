/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/DirectX11Graphics.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DirectX11/DirectX11Graphics.h"
#include "Hyperion/Renderer/DirectX11/DirectX11Buffer.h"
#include "Hyperion/Renderer/DirectX11/DirectX11Texture.h"
#include "Hyperion/Renderer/DirectX11/DirectX11RenderTarget.h"



namespace Hyperion
{

	/*------------------------------------------------------------------------------------------
		DirectX11Graphics Constructor 
	------------------------------------------------------------------------------------------*/
	DirectX11Graphics::DirectX11Graphics()
		: m_bRunning( false ), m_bVSync( false ), m_Resolution()
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
			m_DepthStencilView.Reset();
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
			m_DepthStencilBuffer.Reset();

			D3D11_TEXTURE2D_DESC depthBufferDesc;
			ZeroMemory( &depthBufferDesc, sizeof( depthBufferDesc ) );

			depthBufferDesc.Width				= targetMode.Width;
			depthBufferDesc.Height				= targetMode.Height;
			depthBufferDesc.MipLevels			= 1;
			depthBufferDesc.ArraySize			= 1;
			depthBufferDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
			depthBufferDesc.SampleDesc.Count	= 1;
			depthBufferDesc.SampleDesc.Quality	= 0;
			depthBufferDesc.Usage				= D3D11_USAGE_DEFAULT;
			depthBufferDesc.BindFlags			= D3D11_BIND_DEPTH_STENCIL;
			depthBufferDesc.CPUAccessFlags		= 0;
			depthBufferDesc.MiscFlags			= 0;

			HRESULT res = m_Device->CreateTexture2D( &depthBufferDesc, NULL, m_DepthStencilBuffer.GetAddressOf() );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to update resolution.. couldnt create depth buffer" );
				throw std::exception();
			}

			// Create depth stencil
			m_DepthStencilState.Reset();

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
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to update resolution.. couldnt create depth stencil state" );
				throw std::exception();
			}

			// Set active stencil state
			m_DeviceContext->OMSetDepthStencilState( m_DepthStencilState.Get(), 1 );

			// Create depth stencil view
			D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
			ZeroMemory( &depthViewDesc, sizeof( depthViewDesc ) );

			depthViewDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
			depthViewDesc.ViewDimension			= D3D11_DSV_DIMENSION_TEXTURE2D;
			depthViewDesc.Texture2D.MipSlice	= 0;

			res = m_Device->CreateDepthStencilView( m_DepthStencilBuffer.Get(), &depthViewDesc, m_DepthStencilView.GetAddressOf() );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to update resolution.. couldnt create depth stencil view" );
				throw std::exception();
			}

			// Bind render target view and depth stencil view to the output render view
			ID3D11RenderTargetView *aRenderViews[ 1 ] = { m_RenderTarget->Get() }; // array of pointers
			m_DeviceContext->OMSetRenderTargets( 1, aRenderViews, m_DepthStencilView.Get() );

			// Now update the viewport
			D3D11_VIEWPORT viewport;
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

			// FUTURE: Any other re-init to do here?
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
	bool DirectX11Graphics::Initialize( const IRenderOutput& Output )
	{
		Console::WriteLine( "[STATUS] DX11: Initializing..." );

		if( !Output.Value )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to initialize.. output window invalid!" );
			return false;
		}

		// If any resources are still open.. shut them down
		ShutdownResources();

		// Initialize our resoource using the parameters we have set
		if( !InitializeResources( Output.Value, m_Resolution ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to initialize.. resources couldnt be initialized..." );
			return false;
		}

		m_Output = Output.Value;

		// Generate view matricies based on resolution
		GenerateMatricies( m_Resolution, DirectX::XM_PIDIV4, SCREEN_NEAR, SCREEN_FAR );

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

			// TODO: Multi outputs?
			swapChainDesc.BufferCount			= 2;
			swapChainDesc.BufferDesc			= selectedMode;
			swapChainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.OutputWindow			= Target;
			swapChainDesc.SampleDesc.Count		= 1;
			swapChainDesc.SampleDesc.Quality	= 0;
			swapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapChainDesc.Flags					= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			swapChainDesc.Windowed				= !Resolution.FullScreen;

			// If VSync isnt set, then unlock the refresh rate
			if( !m_bVSync )
			{
				swapChainDesc.BufferDesc.RefreshRate.Numerator		= 0;
				swapChainDesc.BufferDesc.RefreshRate.Denominator	= 1;
			}
			
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
			m_RenderTarget = std::shared_ptr< DirectX11RenderTarget >( new DirectX11RenderTarget( m_BackBuffer ) );
			res = m_Device->CreateRenderTargetView( m_BackBuffer->Get(), NULL, m_RenderTarget->GetAddress() );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create render target view" );
				throw std::exception();
			}

			// Create common states object
			//m_CommonStates = std::make_unique< DirectX::CommonStates >( m_Device.Get() );

			// Create Depth Buffer
			D3D11_TEXTURE2D_DESC depthBufferDesc;
			ZeroMemory( &depthBufferDesc, sizeof( depthBufferDesc ) );

			depthBufferDesc.Width				= selectedMode.Width;
			depthBufferDesc.Height				= selectedMode.Height;
			depthBufferDesc.MipLevels			= 1;
			depthBufferDesc.ArraySize			= 1;
			depthBufferDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
			depthBufferDesc.SampleDesc.Count	= 1;
			depthBufferDesc.SampleDesc.Quality	= 0;
			depthBufferDesc.Usage				= D3D11_USAGE_DEFAULT;
			depthBufferDesc.BindFlags			= D3D11_BIND_DEPTH_STENCIL;
			depthBufferDesc.CPUAccessFlags		= 0;
			depthBufferDesc.MiscFlags			= 0;

			res = m_Device->CreateTexture2D( &depthBufferDesc, NULL, m_DepthStencilBuffer.GetAddressOf() );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create depth buffer" );
				throw std::exception();
			}

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
			m_DeviceContext->OMSetDepthStencilState( m_DepthStencilState.Get(), 1 );

			// Create depth stencil view
			D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
			ZeroMemory( &depthViewDesc, sizeof( depthViewDesc ) );

			depthViewDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
			depthViewDesc.ViewDimension			= D3D11_DSV_DIMENSION_TEXTURE2D;
			depthViewDesc.Texture2D.MipSlice	= 0;

			res = m_Device->CreateDepthStencilView( m_DepthStencilBuffer.Get(), &depthViewDesc, m_DepthStencilView.GetAddressOf() );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create depth stencil view" );
				throw std::exception();
			}

			// Bind render target view and depth stencil view to the output render view
			ID3D11RenderTargetView *aRenderViews[ 1 ] = { m_RenderTarget->Get() }; // array of pointers
			m_DeviceContext->OMSetRenderTargets( 1, aRenderViews, m_DepthStencilView.Get() );

			// Create the rasterizer
			D3D11_RASTERIZER_DESC rasterDesc;
			ZeroMemory( &rasterDesc, sizeof( rasterDesc ) );

			rasterDesc.AntialiasedLineEnable	= false;
			rasterDesc.CullMode					= D3D11_CULL_BACK;
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

		// Clear render target and depth stencil before rendering the next frame
		FLOAT backgroundColor[ 4 ] = { 0.f, 0.f, 0.f, 0.f };
		m_DeviceContext->ClearRenderTargetView( m_RenderTarget->Get(), backgroundColor );
		m_DeviceContext->ClearDepthStencilView( m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.f, 0 );
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
		//if( m_CommonStates )		{ m_CommonStates.reset(); }
		if( m_BackBuffer )			{ m_BackBuffer.reset(); }
		//if( m_EffectFactory )		{ m_EffectFactory->ReleaseCache(); m_EffectFactory.reset(); }
		if( m_RasterizerState )		{ m_RasterizerState.Reset(); }
		if( m_DepthStencilView )	{ m_DepthStencilView.Reset(); }
		if( m_DepthStencilState )	{ m_DepthStencilState.Reset(); }
		if( m_DepthDisabledState )	{ m_DepthDisabledState.Reset(); }
		if( m_DepthStencilBuffer )	{ m_DepthStencilBuffer.Reset(); }
		if( m_BlendState )			{ m_BlendState.Reset(); }
		if( m_BlendDisabledState )	{ m_BlendDisabledState.Reset(); }
		if( m_RenderTarget )		{ m_RenderTarget.reset(); }
		if( m_DeviceContext )		{ m_DeviceContext.Reset(); }
		if( m_Device )				{ m_Device.Reset(); }
		if( m_SwapChain )			{ m_SwapChain->SetFullscreenState( false, nullptr ); m_SwapChain.Reset(); }
	}


	/*------------------------------------------------------------------------------------------
		DirectX11Graphics::GenerateMatricies 
	------------------------------------------------------------------------------------------*/
	void DirectX11Graphics::GenerateMatricies( const ScreenResolution& inRes, float inFOV, float inNear, float inFar )
	{
		// Calculate values needed to generate matricies
		float Aspect = (float) inRes.Width / (float) inRes.Height;

		// Create matricies
		DirectX::XMStoreFloat4x4( &m_ProjectionMatrix, DirectX::XMMatrixPerspectiveFovLH( inFOV, Aspect, inNear, inFar ) );
		DirectX::XMStoreFloat4x4( &m_WorldMatrix, DirectX::XMMatrixIdentity() );
		DirectX::XMStoreFloat4x4( &m_OrthoMatrix, DirectX::XMMatrixOrthographicLH( (float) inRes.Width, (float) inRes.Height, inNear, inFar ) );

		auto up			= DirectX::XMVectorSet( 0.f, 1.f, 0.f, 1.f );
		auto lookAt		= DirectX::XMVectorSet( 0.f, 0.f, 1.f, 1.f );
		auto position	= DirectX::XMFLOAT3( 0.f, 0.f, -0.1f );

		DirectX::XMStoreFloat4x4( &m_ScreenMatrix, DirectX::XMMatrixLookAtLH( DirectX::XMLoadFloat3( &position ), lookAt, up ) );

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


	std::shared_ptr< IRenderTarget > DirectX11Graphics::GetRenderTarget()
	{
		if( m_RenderTarget && m_RenderTarget->IsValid() )
		{
			return m_RenderTarget;
		}

		return nullptr;
	}


	std::shared_ptr< ITexture2D > DirectX11Graphics::GetBackBuffer()
	{
		if( m_BackBuffer && m_BackBuffer->IsValid() )
		{
			return m_BackBuffer;
		}

		return nullptr;
	}


	std::shared_ptr< IBuffer > DirectX11Graphics::CreateBuffer( const BufferParameters& inParams )
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

		return newBuffer;
	}

	// TODO: Aallow multiple bind flags, instead of just a single one
	D3D11_BIND_FLAG TranslateBindFlags( TextureBindTarget inTarget )
	{
		switch( inTarget )
		{
		case TextureBindTarget::Shader:
			return D3D11_BIND_SHADER_RESOURCE;
		case TextureBindTarget::Render:
			return D3D11_BIND_RENDER_TARGET;
		case TextureBindTarget::DepthStencil:
			return D3D11_BIND_DEPTH_STENCIL;
		default:
			return D3D11_BIND_SHADER_RESOURCE;
		}
	}

	std::shared_ptr< ITexture1D > DirectX11Graphics::CreateTexture1D( const Texture1DParameters& inParams )
	{
		if( !m_Device )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 1D texture.. device state was null!" );
			return nullptr;
		}

		// Convert texture parameters into a texture description
		D3D11_TEXTURE1D_DESC Desc;
		ZeroMemory( &Desc, sizeof( Desc ) );

		Desc.Width		= inParams.Width;
		Desc.MipLevels	= 1;
		Desc.ArraySize	= 1;
		Desc.MiscFlags	= 0;

		Desc.Format		= TextureFormatToDXGIFormat( inParams.Format );
		Desc.BindFlags	= TranslateBindFlags( inParams.Target );

		Desc.CPUAccessFlags = 0;
		if( inParams.Dynamic )
		{
			Desc.Usage				= D3D11_USAGE_DYNAMIC;
			Desc.CPUAccessFlags		|= D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			Desc.Usage = D3D11_USAGE_DEFAULT;
		}

		if( inParams.CanCPURead )
		{
			Desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
		}

		// Now that we have the format built, lets create the structure for the starting data (if any)
		D3D11_SUBRESOURCE_DATA Data;
		ZeroMemory( &Data, sizeof( Data ) );

		Data.pSysMem = inParams.Data;

		// Finally, lets create the texture
		std::shared_ptr< DirectX11Texture1D > Output( new DirectX11Texture1D() );
		if( FAILED( m_Device->CreateTexture1D( &Desc, &Data, Output->GetAddress() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 1D texture! API Call failed" );
			return nullptr;
		}
		
		return Output;
	}


	std::shared_ptr< ITexture2D > DirectX11Graphics::CreateTexture2D( const Texture2DParameters& inParams )
	{
		if( !m_Device )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 2D texture.. device state was null!" );
			return nullptr;
		}

		// Convert texture parameters into a texture description
		D3D11_TEXTURE2D_DESC Desc;
		ZeroMemory( &Desc, sizeof( Desc ) );

		Desc.Width			= inParams.Width;
		Desc.Height			= inParams.Height;
		Desc.MipLevels		= inParams.MipLevels;
		Desc.ArraySize		= 1;
		Desc.MiscFlags		= 0;

		Desc.Format		= TextureFormatToDXGIFormat( inParams.Format );
		Desc.BindFlags	= TranslateBindFlags( inParams.Target );

		// TODO: Implement multi-sampling in the generic api interface layer
		Desc.SampleDesc.Count		= 1;
		Desc.SampleDesc.Quality		= 0;

		Desc.CPUAccessFlags = 0;
		if( inParams.Dynamic )
		{
			Desc.Usage = D3D11_USAGE_DYNAMIC;
			Desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			Desc.Usage = D3D11_USAGE_DEFAULT;
		}

		if( inParams.CanCPURead )
		{
			Desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
		}

		// Now that we have the format built, lets create the structure for the starting data (if any)
		std::vector< D3D11_SUBRESOURCE_DATA > DataArray;
		DataArray.resize( inParams.Data.size() );

		for( auto i = 0; i < DataArray.size(); i++ )
		{
			auto& data = DataArray.at( i );
			ZeroMemory( &data, sizeof( data ) );

			auto& source = inParams.Data.at( i );
			
			data.pSysMem			= source.Data;
			data.SysMemPitch		= source.RowDataSize;
			data.SysMemSlicePitch	= 0;
		}

		// Finally, lets create the texture
		std::shared_ptr< DirectX11Texture2D > Output( new DirectX11Texture2D() );
		if( FAILED( m_Device->CreateTexture2D( &Desc, DataArray.data(), Output->GetAddress() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 2D texture! API Call failed" );
			return nullptr;
		}

		return Output;
	}


	std::shared_ptr< ITexture3D > DirectX11Graphics::CreateTexture3D( const Texture3DParameters& inParams )
	{
		if( !m_Device )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 3D texture.. device state was null!" );
			return nullptr;
		}

		// Convert texture parameters into a texture description
		D3D11_TEXTURE3D_DESC Desc;
		ZeroMemory( &Desc, sizeof( Desc ) );

		Desc.Width		= inParams.Width;
		Desc.Height		= inParams.Height;
		Desc.Depth		= inParams.Depth;

		Desc.MipLevels = 1;
		Desc.MiscFlags = 0;

		Desc.Format = TextureFormatToDXGIFormat( inParams.Format );
		Desc.BindFlags = TranslateBindFlags( inParams.Target );

		Desc.CPUAccessFlags = 0;
		if( inParams.Dynamic )
		{
			Desc.Usage = D3D11_USAGE_DYNAMIC;
			Desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			Desc.Usage = D3D11_USAGE_DEFAULT;
		}

		if( inParams.CanCPURead )
		{
			Desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
		}

		// Now that we have the format built, lets create the structure for the starting data (if any)
		D3D11_SUBRESOURCE_DATA Data;
		ZeroMemory( &Data, sizeof( Data ) );

		Data.pSysMem			= inParams.Data;
		Data.SysMemPitch		= inParams.RowDataSize;
		Data.SysMemSlicePitch	= inParams.LayerDataSize;

		// Finally, lets create the texture
		std::shared_ptr< DirectX11Texture3D > Output( new DirectX11Texture3D() );
		if( FAILED( m_Device->CreateTexture3D( &Desc, &Data, Output->GetAddress() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 3D texture! API Call failed" );
			return nullptr;
		}

		return Output;
	}


	bool DirectX11Graphics::CopyTexture2D( std::shared_ptr< ITexture2D >& Source, std::shared_ptr< ITexture2D >& Target )
	{
		// First, verify everything and get the casted pointers we need
		if( !Source || !Source->IsValid() || !Target || !Target->IsValid() )
		{
			Console::WriteLine( "[WARNING] DirectX11: Failed to copy texture, either source or destination was null/invalid" );
			return false;
		}

		// Wish there was a way where we didnt have to perform as many casts and checks for each api call?
		DirectX11Texture2D* SourcePtr = dynamic_cast<DirectX11Texture2D*>( Source.get() );
		DirectX11Texture2D* TargetPtr = dynamic_cast<DirectX11Texture2D*>( Target.get() );

		HYPERION_VERIFY( SourcePtr != nullptr && TargetPtr != nullptr, "Attempt to copy textures from a different graphics api!?" );
		HYPERION_VERIFY( m_DeviceContext, "Attempt to copy textures before initialization or after shutdown?" );

		// Next, call the API function to perform the copy
		m_DeviceContext->CopyResource( SourcePtr->Get(), TargetPtr->Get() );
		return true;
	}

	bool DirectX11Graphics::CopyLODTexture2D( std::shared_ptr< ITexture2D >& Source, std::shared_ptr< ITexture2D >& Dest,
										   uint32 SourceX, uint32 SourceY, uint32 Width, uint32 Height, uint32 DestX, uint32 DestY, uint8 SourceMip, uint8 DestMip )
	{
		if( !Source || !Dest || !Source->IsValid() || !Dest->IsValid() )
		{
			Console::WriteLine( "[WARNING] DirectX11: Failed to copy texture regions, either source or destination was null!" );
			return false;
		}

		DirectX11Texture2D* SourcePtr = dynamic_cast< DirectX11Texture2D* >( Source.get() );
		DirectX11Texture2D* DestPtr = dynamic_cast< DirectX11Texture2D* >( Dest.get() );

		HYPERION_VERIFY( SourcePtr != nullptr && DestPtr != nullptr, "Attempt to copy textures from a different graphics api?" );
		HYPERION_VERIFY( m_DeviceContext, "Attempt to copy textures before initialization or after shutdown?" );

		// Next, could perform further validation, but lets just go ahead for now
		D3D11_BOX Box;
		
		Box.left	= SourceX;
		Box.right	= SourceX + Width;
		Box.top		= SourceY;
		Box.bottom	= SourceY + Height;
		Box.front	= 0;
		Box.back	= 1;
		
		m_DeviceContext->CopySubresourceRegion( DestPtr->Get(), DestMip, DestX, DestY, 0, SourcePtr->Get(), SourceMip, &Box );
		return true;
	}


	std::shared_ptr< IRenderTarget > DirectX11Graphics::CreateRenderTarget( std::shared_ptr< ITexture2D > inTarget )
	{
		if( !m_Device )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create render target! Device was null!" );
			return nullptr;
		}

		// Validate the texture parameter
		if( !inTarget || !inTarget->IsValid() || ( (int)inTarget->GetBindTarget() & (int)TextureBindTarget::Render ) == 0 )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create render target! Source texture was invalid.. or wasnt bound properly" );
			return nullptr;
		}

		// Get the actual DX11 texture pointer from the texture
		auto dx11tex = std::dynamic_pointer_cast<DirectX11Texture2D>( inTarget );
		HYPERION_VERIFY( dx11tex != nullptr, "Invalid resource type! Expected DX11 texture, got other type?" );

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory( &rtvDesc, sizeof( rtvDesc ) );

		// We need to get the texture format
		auto fmt = TextureFormatToDXGIFormat( dx11tex->GetFormat() );
		if( fmt == DXGI_FORMAT_UNKNOWN )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create render target! Source texture had an unknown format" );
			return nullptr;
		}
		
		rtvDesc.Format				= fmt;
		rtvDesc.ViewDimension		= D3D11_RTV_DIMENSION_TEXTURE2D; // TODO: Enable different view dimenions
		rtvDesc.Texture2D.MipSlice	= 0;

		auto output = std::shared_ptr< DirectX11RenderTarget >( new DirectX11RenderTarget( inTarget ) );

		auto res = m_Device->CreateRenderTargetView( dx11tex->Get(), &rtvDesc, output->GetAddress() );
		if( FAILED( res ) || !output->IsValid() )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create render target! DX11 API failed to create the instance" );
			return nullptr;
		}

		return output;
	}

}