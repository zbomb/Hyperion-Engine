/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/DX11Graphics.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DirectX11/DX11Graphics.h"
#include "Hyperion/Renderer/DirectX11/DX11Texture.h"
#include "Hyperion/Renderer/DirectX11/DX11Buffer.h"

/*
#include "Hyperion/Renderer/DirectX11/DirectX11Buffer.h"
#include "Hyperion/Renderer/DirectX11/DirectX11Texture.h"
#include "Hyperion/Renderer/DirectX11/DirectX11RenderTarget.h"
#include "Hyperion/Library/Geometry.h"
#include "Hyperion/Renderer/DirectX11/DirectX11DepthStencil.h"
#include "Hyperion/Renderer/GBuffer.h"
#include "Hyperion/Renderer/DirectX11/DirectX11LightBuffer.h"
#include "Hyperion/Renderer/DirectX11/DirectX11ViewClusters.h"
*/

/*
*	Shaders
*/
#include "Hyperion/Renderer/DirectX11/Shaders/DX11CullLightsShader.h"
#include "Hyperion/Renderer/DirectX11/Shaders/DX11FindClustersShader.h"
#include "Hyperion/Renderer/DirectX11/Shaders/DX11BuildClustersShader.h"
#include "Hyperion/Renderer/DirectX11/Shaders/DX11GBufferPixelShader.h"
#include "Hyperion/Renderer/DirectX11/Shaders/DX11LightingPixelShader.h"
#include "Hyperion/Renderer/DirectX11/Shaders/DX11SceneVertexShader.h"
#include "Hyperion/Renderer/DirectX11/Shaders/DX11ScreenVertexShader.h"
#include "Hyperion/Renderer/DirectX11/Shaders/DX11ForwardPreZShader.h"
#include "Hyperion/Renderer/DirectX11/Shaders/DX11ForwardPixelShader.h"
#include "Hyperion/Renderer/DirectX11/Shaders/DX11FXAAShader.h"


namespace Hyperion
{

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::DX11Graphics
	--------------------------------------------------------------------------------------------*/
	DX11Graphics::DX11Graphics()
		: m_OSTarget( nullptr ), m_Resolution(), m_bVSync( false ), m_AvailableResolutions(), m_DepthStencilWidth( 0 ), m_DepthStencilHeight( 0 ), 
		m_GraphicsDeviceName(), m_DedicatedVideoMemory( 0 ), m_SharedVideoMemory( 0 ), m_DisplayRefreshRate(), m_AlphaBlendState( AlphaBlendingState::Disabled ),
		m_DepthState( DepthStencilState::DepthAndStencilEnabled ), m_bAsyncResourceCreation( false ), m_AttachedIndexBuffer( nullptr ),
		m_AttachedVertexBuffer( nullptr ), m_FloorVertexBuffer( nullptr ), m_FloorIndexBuffer( nullptr )
	{

	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::~DX11Graphics
	--------------------------------------------------------------------------------------------*/
	DX11Graphics::~DX11Graphics()
	{
		Shutdown();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::Initialize
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::Initialize( const InitializationParameters& inParameters )
	{
		// Verify that we are not already running
		HYPERION_VERIFY( m_OSTarget == nullptr && m_Device == nullptr && m_Context == nullptr, "[DX11] Attempt to initialize, when already running" );

		// Check if were attempting to make a fullscreen window, if so, then we are going to grab the desktop size
		ScreenResolution targetResolution	= inParameters.startupResolution;
		uint32 depthStencilWidth			= inParameters.depthStencilWidth;
		uint32 depthStencilHeight			= inParameters.depthStencilHeight;
		String graphicsCardName				= "";
		uint64 dedicatedGraphicsMemory		= 0;
		uint64 sharedGraphicsMemory			= 0;

		if( targetResolution.FullScreen )
		{
			RECT desktopRect {};
			GetWindowRect( GetDesktopWindow(), &desktopRect );

			targetResolution.Width		= (uint32) desktopRect.right;
			targetResolution.Height		= (uint32) desktopRect.bottom;

			depthStencilWidth	= Math::Max( depthStencilWidth, targetResolution.Width );
			depthStencilHeight	= Math::Max( depthStencilHeight, targetResolution.Height );
		}

		// Validate the parameters
		std::vector< ScreenResolution > resolutions;
		GetAvailableResolutions( resolutions );

		bool bValidResolution = false;

		for( int i = 0; i < resolutions.size(); i++ )
		{
			if( inParameters.startupResolution.Width == resolutions[ i ].Width &&
				inParameters.startupResolution.Height == resolutions[ i ].Height )
			{
				bValidResolution = true;
				break;
			}
		}

		if( !bValidResolution || inParameters.ptrOSWindow == nullptr ||
			depthStencilWidth < targetResolution.Width ||
			depthStencilHeight < targetResolution.Height )
		{
			Console::WriteLine( "[DX11] ERROR: Failed to initialize, invalid parameters provided" );
			return false;
		}

		// Start creating our resources
		try
		{
			ComPtr< IDXGIFactory1 > dxFactory;
			ComPtr< IDXGIAdapter1 > dxAdapter;
			ComPtr< IDXGIOutput > dxOutput;
			ComPtr< ID3D11Device > dxDevice;
			ComPtr< ID3D11DeviceContext > dxContext;
			ComPtr< IDXGISwapChain > dxSwapChain;

			// Start with the factory..
			if( FAILED( CreateDXGIFactory1( __uuidof( IDXGIFactory1 ), (void**)dxFactory.GetAddressOf() ) ) )
			{
				throw std::exception( "failed to create DXGI factory" );
			}

			DXGI_ADAPTER_DESC1 gpuDesc {};

			// Get the video output
			// TODO: Allow output device switching/selecting
			if( FAILED( dxFactory->EnumAdapters1( 0, dxAdapter.GetAddressOf() ) ) ||
				FAILED( dxAdapter->EnumOutputs( 0, dxOutput.GetAddressOf() ) ) ||
				FAILED( dxAdapter->GetDesc1( &gpuDesc ) ) )
			{
				throw std::exception( "failed to get output/adapter" );
			}

			// Store some info about the GPU and Vmem
			graphicsCardName			= String( gpuDesc.Description, StringEncoding::UTF16 );
			dedicatedGraphicsMemory		= (uint64) gpuDesc.DedicatedVideoMemory;
			sharedGraphicsMemory		= (uint64) gpuDesc.SharedSystemMemory;

			// Ensure the window size matches the resolution were targetting
			HWND windowHandle = static_cast<HWND>( inParameters.ptrOSWindow );
			ResizeWindow( windowHandle, targetResolution.Width, targetResolution.Height );

			// Create the DX11 device
			D3D_FEATURE_LEVEL targetFeatureLevels[] = { D3D_FEATURE_LEVEL_11_1 };
			D3D_FEATURE_LEVEL loadedFeatureLevel;

		#ifdef HYPERION_DEBUG_RENDERER
			UINT deviceFlags = D3D11_CREATE_DEVICE_DEBUG;
		#else
			UINT deviceFlags = 0;
		#endif

			if( FAILED( D3D11CreateDevice( dxAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, deviceFlags, targetFeatureLevels, 1, D3D11_SDK_VERSION, dxDevice.GetAddressOf(), &loadedFeatureLevel, dxContext.GetAddressOf() ) ) ||
				FAILED( dxDevice->QueryInterface< ID3D11Device2 >( m_Device.ReleaseAndGetAddressOf() ) ) ||
				FAILED( dxContext->QueryInterface< ID3D11DeviceContext2 >( m_Context.ReleaseAndGetAddressOf() ) ) ||
				loadedFeatureLevel != D3D_FEATURE_LEVEL_11_1 )
			{
				throw std::exception( "failed to create device and context" );
			}

			// Query for IDXGIDevice so we can set the max number of pre-rendered frames
			ComPtr< IDXGIDevice1 > dxgiDevice;
			if( FAILED( m_Device->QueryInterface< IDXGIDevice1 >( dxgiDevice.ReleaseAndGetAddressOf() ) ) )
			{
				dxgiDevice->SetMaximumFrameLatency( 2 );
			}

			// Create the swap chain
			DXGI_SWAP_CHAIN_DESC swapChainDesc {};

			swapChainDesc.BufferCount			= 2;
			swapChainDesc.BufferDesc.Width		= targetResolution.Width;
			swapChainDesc.BufferDesc.Height		= targetResolution.Height;
			swapChainDesc.BufferDesc.Format		= DXGI_FORMAT_R8G8B8A8_UNORM; // We use a non-SRGB format, but the render target view is SRGB, to enable auto-format switching
			swapChainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.OutputWindow			= windowHandle;
			swapChainDesc.SampleDesc.Count		= 1;
			swapChainDesc.SampleDesc.Quality	= 0;
			swapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapChainDesc.Flags					= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			swapChainDesc.Windowed				= FALSE;

			// TODO: Determine if we should use the display modes actual refresh rate here and enforce VSync through the 'Present' function
			//swapChainDesc.BufferDesc.RefreshRate.Numerator		= 0;
			//swapChainDesc.BufferDesc.RefreshRate.Denominator	= inParameters.bEnableVSync ? 1 : 0;
			swapChainDesc.BufferDesc.RefreshRate = m_DisplayRefreshRate;

			if( FAILED( dxFactory->CreateSwapChain( m_Device.Get(), &swapChainDesc, dxSwapChain.GetAddressOf() ) ) ||
				FAILED( dxSwapChain->QueryInterface< IDXGISwapChain2 >( m_SwapChain.ReleaseAndGetAddressOf() ) ) )
			{
				throw std::exception( "failed to create swap chain" );
			}

			// Change to fullscreen if required
			if( targetResolution.FullScreen )
			{
				if( FAILED( m_SwapChain->SetFullscreenState( TRUE, dxOutput.Get() ) ) )
				{
					throw std::exception( "failed to enable fullscreen mode" );
				}
			}

			auto backBuffer = std::shared_ptr< DX11Texture2D >( new DX11Texture2D() );

			// Create our back buffer render target view
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc {};

			rtvDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			rtvDesc.Texture2D.MipSlice	= 0;

			if( FAILED( m_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**) backBuffer->GetAddress() ) ) ||
				FAILED( m_Device->CreateRenderTargetView( backBuffer->Get(), &rtvDesc, backBuffer->GetRTVAddress() ) ) )
			{
				throw std::exception( "failed to create back buffer render target" );
			}

			m_BackBuffer = backBuffer;

			// Create depth stencil states
			D3D11_DEPTH_STENCIL_DESC depthStencilDesc {};

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

			if( FAILED( m_Device->CreateDepthStencilState( &depthStencilDesc, m_DepthEnabledState.ReleaseAndGetAddressOf() ) ) )
			{
				throw std::exception( "Failed to create depth stencil state" );
			}

			// Create depth stencil state for when depth is disabled
			depthStencilDesc.DepthEnable		= false;
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

			if( FAILED( m_Device->CreateDepthStencilState( &depthStencilDesc, m_DepthDisabledState.ReleaseAndGetAddressOf() ) ) )
			{
				throw std::exception( "Failed to create depth stencil state" );
			}

			// Create the actual depth stencil
			D3D11_TEXTURE2D_DESC depthBufferDesc {};

			depthBufferDesc.Width			= depthStencilWidth;
			depthBufferDesc.Height			= depthStencilHeight;
			depthBufferDesc.Format			= DXGI_FORMAT_D24_UNORM_S8_UINT;
			depthBufferDesc.BindFlags		= D3D11_BIND_DEPTH_STENCIL;
			depthBufferDesc.Usage			= D3D11_USAGE_DEFAULT;
			depthBufferDesc.ArraySize		= 1;
			depthBufferDesc.MipLevels		= 1;
			depthBufferDesc.CPUAccessFlags	= 0;
			depthBufferDesc.MiscFlags		= 0;

			D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc {};

			depthViewDesc.ViewDimension			= D3D11_DSV_DIMENSION_TEXTURE2D;
			depthViewDesc.Flags					= 0;
			depthViewDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
			depthViewDesc.Texture2D.MipSlice	= 0;

			if( FAILED( m_Device->CreateTexture2D( &depthBufferDesc, NULL, m_DepthBuffer.ReleaseAndGetAddressOf() ) ) ||
				FAILED( m_Device->CreateDepthStencilView( m_DepthBuffer.Get(), &depthViewDesc, m_DepthStencilView.ReleaseAndGetAddressOf() ) ) )
			{
				throw std::exception( "failed to create depth buffer/view" );
			}

			// Create the rasterizer state
			D3D11_RASTERIZER_DESC1 rasterDesc {};

			rasterDesc.AntialiasedLineEnable	= false;
			rasterDesc.CullMode					= D3D11_CULL_BACK;
			rasterDesc.DepthBias				= 0;
			rasterDesc.DepthBiasClamp			= 0.f;
			rasterDesc.DepthClipEnable			= false;
			rasterDesc.FillMode					= D3D11_FILL_SOLID;
			rasterDesc.FrontCounterClockwise	= false;
			rasterDesc.MultisampleEnable		= false;
			rasterDesc.ScissorEnable			= false;
			rasterDesc.SlopeScaledDepthBias		= 0.f;
			rasterDesc.ForcedSampleCount		= 0;

			if( FAILED( m_Device->CreateRasterizerState1( &rasterDesc, m_RasterState.ReleaseAndGetAddressOf() ) ) )
			{
				throw std::exception( "failed to create raster state" );
			}

			// Create blend states
			D3D11_BLEND_DESC blendEnabledDesc {};

			blendEnabledDesc.RenderTarget[ 0 ].BlendEnable				= TRUE;
			blendEnabledDesc.RenderTarget[ 0 ].SrcBlend					= D3D11_BLEND_SRC_ALPHA;
			blendEnabledDesc.RenderTarget[ 0 ].DestBlend				= D3D11_BLEND_INV_SRC_ALPHA;
			blendEnabledDesc.RenderTarget[ 0 ].BlendOp					= D3D11_BLEND_OP_ADD;
			blendEnabledDesc.RenderTarget[ 0 ].SrcBlendAlpha			= D3D11_BLEND_ONE;
			blendEnabledDesc.RenderTarget[ 0 ].DestBlendAlpha			= D3D11_BLEND_ZERO;
			blendEnabledDesc.RenderTarget[ 0 ].BlendOpAlpha				= D3D11_BLEND_OP_ADD;
			blendEnabledDesc.RenderTarget[ 0 ].RenderTargetWriteMask	= 0x0f;

			D3D11_BLEND_DESC blendDisabledDesc {};

			blendDisabledDesc.RenderTarget[ 0 ].BlendEnable				= FALSE;
			blendDisabledDesc.RenderTarget[ 0 ].SrcBlend				= D3D11_BLEND_ONE;
			blendDisabledDesc.RenderTarget[ 0 ].DestBlend				= D3D11_BLEND_INV_SRC_ALPHA;
			blendDisabledDesc.RenderTarget[ 0 ].BlendOp					= D3D11_BLEND_OP_ADD;
			blendDisabledDesc.RenderTarget[ 0 ].SrcBlendAlpha			= D3D11_BLEND_ONE;
			blendDisabledDesc.RenderTarget[ 0 ].DestBlendAlpha			= D3D11_BLEND_ZERO;
			blendDisabledDesc.RenderTarget[ 0 ].BlendOpAlpha			= D3D11_BLEND_OP_ADD;
			blendDisabledDesc.RenderTarget[ 0 ].RenderTargetWriteMask	= 0x0f;

			if( FAILED( m_Device->CreateBlendState( &blendEnabledDesc, m_BlendEnabledState.ReleaseAndGetAddressOf() ) ) ||
				FAILED( m_Device->CreateBlendState( &blendDisabledDesc, m_BlendDisabledState.ReleaseAndGetAddressOf() ) ) )
			{
				throw std::exception( "failed to create blend states" );
			}
		}
		catch( std::exception& ex )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to initialize! ", ex.what() );
			Shutdown();
			return false;
		}

		// Now, were going to set some of the static pipeline state, or the pipeline state that doesnt change very often
		// Create the default viewport
		D3D11_VIEWPORT viewport {};

		viewport.Width		= (float) targetResolution.Width;
		viewport.Height		= (float) targetResolution.Height;
		viewport.MinDepth	= 0.f;
		viewport.MaxDepth	= 1.f;
		viewport.TopLeftX	= 0.f;
		viewport.TopLeftY	= 0.f;

		D3D11_VIEWPORT viewportList[] = { viewport };
		m_Context->RSSetViewports( 1, viewportList );
		m_Context->RSSetState( m_RasterState.Get() );

		SetAlphaBlendingState( AlphaBlendingState::Disabled );
		SetDepthStencilState( DepthStencilState::DepthAndStencilEnabled );

		// Store initialization parameters
		m_bVSync				= inParameters.bEnableVSync;
		m_Resolution			= targetResolution;
		m_OSTarget				= static_cast<HWND>( inParameters.ptrOSWindow );
		m_DepthStencilWidth		= depthStencilWidth;
		m_DepthStencilHeight	= depthStencilHeight;
		m_GraphicsDeviceName	= graphicsCardName;
		m_DedicatedVideoMemory	= dedicatedGraphicsMemory;
		m_SharedVideoMemory		= sharedGraphicsMemory;

		// Check feature support
		D3D11_FEATURE_DATA_THREADING threading {};
		m_Device->CheckFeatureSupport( D3D11_FEATURE_THREADING, (void*) &threading, sizeof( threading ) );

		m_bAsyncResourceCreation		= threading.DriverConcurrentCreates;

		// Generate our screen quad for the selected resolution
		RebuildScreenQuad( m_Resolution.Width, m_Resolution.Height );
		RebuildDebugFloorBuffers();

		return true;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::Shutdown
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::Shutdown()
	{
		m_FloorIndexBuffer.reset();
		m_FloorVertexBuffer.reset();
		m_AttachedIndexBuffer.reset();
		m_AttachedVertexBuffer.reset();
		m_ScreenQuadVertexBuffer.Reset();
		m_AttachedRenderTargets.clear();
		m_BlendDisabledState.Reset();
		m_BlendEnabledState.Reset();
		m_RasterState.Reset();
		m_DepthStencilView.Reset();
		m_DepthBuffer.Reset();
		m_DepthEnabledState.Reset();
		m_DepthDisabledState.Reset();
		m_BackBufferRenderTarget.Reset();
		m_BackBuffer.reset();
		m_Debug.Reset();
		m_SwapChain.Reset();
		m_Context.Reset();
		m_Device.Reset();

		// Clear members
		m_OSTarget					= nullptr;
		m_bVSync					= false;
		m_Resolution.FullScreen		= false;
		m_Resolution.Width			= 0;
		m_Resolution.Height			= 0;
		m_DepthStencilWidth			= 0;
		m_DepthStencilHeight		= 0;
		m_DedicatedVideoMemory		= 0;
		m_SharedVideoMemory			= 0;

		m_DisplayRefreshRate.Numerator		= 0;
		m_DisplayRefreshRate.Denominator	= 0;

		m_GraphicsDeviceName.Clear();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::ResizeWindow
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::ResizeWindow( HWND inWindow, uint32 inWidth, uint32 inHeight )
	{
		// Set window size to our resolution
		RECT newSize = { 0, 0, (long) inWidth, (long) inHeight };
		if( !AdjustWindowRectEx( &newSize, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to update resolution.. couldnt resize window when exiting fullscreen" );
			return;
		}

		// Center the window
		SetWindowPos( inWindow, HWND_TOP, 0, 0, newSize.right - newSize.left, newSize.bottom - newSize.top, SWP_NOMOVE );
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::UpdateResolution
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::UpdateResolution( const ScreenResolution& inResolution )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null" );

		// Ensure an actual change needs to be made
		if( inResolution.FullScreen == m_Resolution.FullScreen &&
			inResolution.Width == m_Resolution.Width && inResolution.Height == m_Resolution.Height )
		{
			return true;
		}

		uint32 selectedWidth	= inResolution.Width;
		uint32 selectedHeight	= inResolution.Height;

		if( inResolution.FullScreen )
		{
			// Get the desired resolution from the size of the desktop for now
			RECT desktopSize {};
			if( !GetWindowRect( GetDesktopWindow(), &desktopSize ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to enter fullscreen mode, couldnt get desktop size from Win32!" );
				return false;
			}

			selectedWidth	= (uint32) desktopSize.right;
			selectedHeight	= (uint32) desktopSize.bottom;
		}

		if( selectedWidth < RENDERER_MIN_RESOLUTION_WIDTH ||
			selectedHeight < RENDERER_MIN_RESOLUTION_HEIGHT )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to update resolution, the selected resolution was less than the minimum allowed" );
			return false;
		}

		uint32 newDepthStencilWidth		= Math::Max( m_DepthStencilWidth, selectedWidth );
		uint32 newDepthStencilHeight	= Math::Max( m_DepthStencilHeight, selectedHeight );

		// Ensure render targets are cleared
		DetachRenderTargets();

		// Flush the pipeline
		m_Context->Flush();

		// Release resources that we need to re-create
		m_BackBufferRenderTarget.Reset();
		m_BackBuffer.reset();
		m_DepthStencilView.Reset();
		m_DepthBuffer.Reset();

		// Resize the window
		ResizeWindow( m_OSTarget, selectedWidth, selectedHeight );

		// Resize the swap chain
		if( FAILED( m_SwapChain->ResizeBuffers( 2, selectedWidth, selectedHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 0 ) ) )
		{
			Console::WriteLine( "[WARNING] DX11: Failed to update resolution, the swap chain buffers couldnt be resized" );

			// We need to restart everything to recover from this error
			InitializationParameters params {};

			params.ptrOSWindow						= m_OSTarget;
			params.bEnableVSync						= m_bVSync;
			params.startupResolution.Width			= selectedWidth;
			params.startupResolution.Height			= selectedHeight;
			params.startupResolution.FullScreen		= inResolution.FullScreen;
			params.depthStencilWidth				= newDepthStencilWidth;
			params.depthStencilHeight				= newDepthStencilHeight;

			Shutdown();
			if( !Initialize( params ) )
			{
				Console::WriteLine( "[FATAL] DX11: Failed to recover from swap chain error!" );
				MessageBox( m_OSTarget, L"Hyperion Engine Fatal Error!", L"Failed to recover from a swap chain error during resize", MB_OK );
				std::terminate();
				return false;
			}
			else
			{
				return true;
			}
		}

		// Reaquire the back buffer and re-create the render target view
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc {};

		rtvDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension		= D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice	= 0;

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc {};

		dsvDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Flags				= 0;
		dsvDesc.ViewDimension		= D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice	= 0;

		D3D11_TEXTURE2D_DESC bufferDesc {};

		bufferDesc.Width			= newDepthStencilWidth;
		bufferDesc.Height			= newDepthStencilHeight;
		bufferDesc.Format			= DXGI_FORMAT_D24_UNORM_S8_UINT;
		bufferDesc.BindFlags		= D3D11_BIND_DEPTH_STENCIL;
		bufferDesc.Usage			= D3D11_USAGE_DEFAULT;
		bufferDesc.ArraySize		= 1;
		bufferDesc.MipLevels		= 1;
		bufferDesc.CPUAccessFlags	= 0;
		bufferDesc.MiscFlags		= 0;

		auto backBuffer = std::shared_ptr< DX11Texture2D >( new DX11Texture2D() );

		if( FAILED( m_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**) backBuffer->GetAddress() ) ) ||
			FAILED( m_Device->CreateRenderTargetView( backBuffer->Get(), &rtvDesc, backBuffer->GetRTVAddress() ) ) ||
			FAILED( m_Device->CreateTexture2D( &bufferDesc, NULL, m_DepthBuffer.ReleaseAndGetAddressOf() ) ) ||
			FAILED( m_Device->CreateDepthStencilView( m_DepthBuffer.Get(), &dsvDesc, m_DepthStencilView.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[FATAL] DX11: Failed to resize main resources" );
			MessageBox( m_OSTarget, L"Hyperion Engine Fatal Error!", L"Failed to resize engine resources!", MB_OK );
			std::terminate();
			return false;
		}

		m_BackBuffer = backBuffer;

		D3D11_VIEWPORT viewport {};

		viewport.Width		= (FLOAT) selectedWidth;
		viewport.Height		= (FLOAT) selectedHeight;
		viewport.TopLeftX	= 0;
		viewport.TopLeftY	= 0;
		viewport.MinDepth	= 0.f;
		viewport.MaxDepth	= 1.f;

		D3D11_VIEWPORT viewList[] = { viewport };
		m_Context->RSSetViewports( 1, viewList );

		m_Resolution.Width			= selectedWidth;
		m_Resolution.Height			= selectedHeight;
		m_Resolution.FullScreen		= inResolution.FullScreen;
		m_DepthStencilWidth			= newDepthStencilWidth;
		m_DepthStencilHeight		= newDepthStencilHeight;

		// Resize any additional resources...
		RebuildScreenQuad( selectedWidth, selectedHeight );

		Console::WriteLine( "Engine: Resized output to ", selectedWidth, "x", selectedHeight );

		return true;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::GetResolution
	--------------------------------------------------------------------------------------------*/
	ScreenResolution DX11Graphics::GetResolution()
	{
		return m_Resolution;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::GetAvailableResolutions
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::GetAvailableResolutions( std::vector<ScreenResolution>& outResolutions )
	{
		static std::vector< ScreenResolution > output {};

		if( output.size() == 0 )
		{
		// Query DXGI to get the resolutions that are currently available on the main output
		// TODO: Allow selection of display
			ComPtr< IDXGIFactory1 > dxFactory;
			ComPtr< IDXGIAdapter1 > dxAdapter;
			ComPtr< IDXGIOutput > dxOutput;

			if( FAILED( CreateDXGIFactory1( __uuidof( IDXGIFactory1 ), (void**) dxFactory.GetAddressOf() ) ) ||
				FAILED( dxFactory->EnumAdapters1( 0, dxAdapter.GetAddressOf() ) ) ||
				FAILED( dxAdapter->EnumOutputs( 0, dxOutput.GetAddressOf() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to get list of available resolutions, DXGI calls failed" );
				return;
			}

			UINT displayModeCount = 0;
			if( FAILED( dxOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 0, &displayModeCount, nullptr ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to get list of available resolutions, failed to get display mode list" );
				return;
			}

			std::vector< DXGI_MODE_DESC > modeList;
			modeList.resize( displayModeCount );

			if( FAILED( dxOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 0, &displayModeCount, modeList.data() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to get list of available resolutions, failed to get display mode list (2)" );
				return;
			}

			float highestRefreshRate = 0.f;
			for( auto it = modeList.begin(); it != modeList.end(); it++ )
			{
				ScreenResolution res {};

				res.Width		= it->Width;
				res.Height		= it->Height;
				res.FullScreen	= false;

				output.emplace_back( res );

				float fRefreshRate = (float) it->RefreshRate.Numerator / (float) it->RefreshRate.Denominator;
				if( fRefreshRate > highestRefreshRate )
				{
					m_DisplayRefreshRate = it->RefreshRate;
				}
			}
		}

		outResolutions = output;
	}

	void DX11Graphics::RebuildDebugFloorBuffers()
	{
		Vertex3D verts[ 4 ] {};

		float texRepeatDistance = 10.f;

		// Top Left
		verts[ 0 ].x	= 0.f;
		verts[ 0 ].y	= 0.f;
		verts[ 0 ].z	= 500.f;
		verts[ 0 ].nx	= 0.f;
		verts[ 0 ].ny	= 1.f;
		verts[ 0 ].nz	= 0.f;
		verts[ 0 ].u	= 0.f;
		verts[ 0 ].v	= 0.f;

		// Top Right
		verts[ 1 ].x	= 500;
		verts[ 1 ].y	= 0.f;
		verts[ 1 ].z	= 500;
		verts[ 1 ].nx	= 0.f;
		verts[ 1 ].ny	= 1.f;
		verts[ 1 ].nz	= 0.f;
		verts[ 1 ].u	= 25.f;
		verts[ 1 ].v	= 0.f;

		// Bottom Left
		verts[ 2 ].x	= 0.f;
		verts[ 2 ].y	= 0.f;
		verts[ 2 ].z	= 0.f;
		verts[ 2 ].nx	= 0.f;
		verts[ 2 ].ny	= 1.f;
		verts[ 2 ].nz	= 0.f;
		verts[ 2 ].u	= 0.f;
		verts[ 2 ].v	= 25.f;

		// Bottom Right
		verts[ 3 ].x	= 500.f;
		verts[ 3 ].y	= 0.f;
		verts[ 3 ].z	= 0.f;
		verts[ 3 ].nx	= 0.f;
		verts[ 3 ].ny	= 1.f;
		verts[ 3 ].nz	= 0.f;
		verts[ 3 ].u	= 25.f;
		verts[ 3 ].v	= 25.f;

		uint32 indexList[] = { 0, 3, 2, 0, 1, 3 };

		auto vertBuffer		= std::shared_ptr< DX11Buffer >( new DX11Buffer( BufferType::Vertex ) );
		auto indexBuffer	= std::shared_ptr< DX11Buffer >( new DX11Buffer( BufferType::Index ) );

		D3D11_BUFFER_DESC vertDesc;
		ZeroMemory( &vertDesc, sizeof( vertDesc ) );

		vertDesc.Usage					= D3D11_USAGE_DEFAULT;
		vertDesc.ByteWidth				= sizeof( Vertex3D ) * 4;
		vertDesc.BindFlags				= D3D11_BIND_VERTEX_BUFFER;
		vertDesc.CPUAccessFlags			= 0;
		vertDesc.MiscFlags				= 0;
		vertDesc.StructureByteStride	= 0;

		D3D11_SUBRESOURCE_DATA vertexData;
		ZeroMemory( &vertexData, sizeof( vertexData ) );

		vertexData.pSysMem				= (void*) verts;
		vertexData.SysMemPitch			= 0;
		vertexData.SysMemSlicePitch		= 0;

		if( FAILED( m_Device->CreateBuffer( &vertDesc, &vertexData, vertBuffer->GetAddress() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create floor vertex list!" );
			return;
		}

		vertBuffer->m_AssetIdentifier	= ASSET_INVALID;
		vertBuffer->m_ElementSize		= sizeof( Vertex3D );
		vertBuffer->m_ElementCount		= 4;
		vertBuffer->m_TotalSize			= sizeof( Vertex3D ) * 4;
		
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

		if( FAILED( m_Device->CreateBuffer( &indexDesc, &indexData, indexBuffer->GetAddress() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create floor index list!" );
			return;
		}

		indexBuffer->m_AssetIdentifier	= ASSET_INVALID;
		indexBuffer->m_ElementSize		= sizeof( UINT );
		indexBuffer->m_ElementCount		= 6;
		indexBuffer->m_TotalSize		= sizeof( UINT ) * 6;

		m_FloorMatricies.clear();
		for( int x = 0; x < 20; x++ )
		{
			for( int y = 0; y < 20; y++ )
			{
				m_FloorMatricies.push_back( Matrix( DirectX::XMMatrixTranslation( -5000.f + ( x * 500 ), 0.f, -5000.f + ( y * 500 ) ).r[ 0 ].m128_f32 ) );
			}
		}

		m_FloorVertexBuffer		= vertBuffer;
		m_FloorIndexBuffer		= indexBuffer;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::GetDisplayRefreshRate
	--------------------------------------------------------------------------------------------*/
	float DX11Graphics::GetDisplayRefreshRate()
	{
		// Check if we queried for available resolutions and refresh rates already
		// If not, we will make a call to that function (although we wont be making use of the output)
		if( m_AvailableResolutions.size() == 0 )
		{
			std::vector< ScreenResolution > resList;
			GetAvailableResolutions( resList );
		}

		return (float) m_DisplayRefreshRate.Numerator / (float) m_DisplayRefreshRate.Denominator;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::GetDepthStencilResolution
	--------------------------------------------------------------------------------------------*/
	std::pair<uint32, uint32> DX11Graphics::GetDepthStencilResolution()
	{
		return std::make_pair( m_DepthStencilWidth, m_DepthStencilHeight );
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::SetDepthStencilResolution
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::SetDepthStencilResolution( uint32 inWidth, uint32 inHeight )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null" );

		uint32 finalWidth	= Math::Max( inWidth, m_Resolution.Width );
		uint32 finalHeight	= Math::Max( inHeight, m_Resolution.Height );

		if( finalWidth == m_DepthStencilWidth && finalHeight == m_DepthStencilHeight )
		{
			return true;
		}

		// Now, we need to resize the depth buffer
		// First, detach the depth stencil 
		ID3D11RenderTargetView* rtvList[] = { nullptr, nullptr, nullptr };
		m_Context->OMSetRenderTargets( 3, rtvList, nullptr );

		m_DepthBuffer.Reset();
		m_DepthStencilView.Reset();

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc {};

		dsvDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Flags				= 0;
		dsvDesc.ViewDimension		= D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice	= 0;

		D3D11_TEXTURE2D_DESC bufferDesc {};

		bufferDesc.Width			= finalWidth;
		bufferDesc.Height			= finalHeight;
		bufferDesc.Format			= DXGI_FORMAT_D24_UNORM_S8_UINT;
		bufferDesc.BindFlags		= D3D11_BIND_DEPTH_STENCIL;
		bufferDesc.Usage			= D3D11_USAGE_DEFAULT;
		bufferDesc.ArraySize		= 1;
		bufferDesc.MipLevels		= 1;
		bufferDesc.CPUAccessFlags	= 0;
		bufferDesc.MiscFlags		= 0;

		if( FAILED( m_Device->CreateTexture2D( &bufferDesc, NULL, m_DepthBuffer.ReleaseAndGetAddressOf() ) ) ||
			FAILED( m_Device->CreateDepthStencilView( m_DepthBuffer.Get(), &dsvDesc, m_DepthStencilView.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to resize depth buffer!" );
			return false;
		}

		m_DepthStencilWidth		= finalWidth;
		m_DepthStencilHeight	= finalHeight;

		return true;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::ClearDepthStencil
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::ClearDepthStencil( float inDepth, uint8 inStencil )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null" );

		m_Context->ClearDepthStencilView( m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, inDepth, inStencil );
		return true;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::GetVSyncEnabled
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::GetVSyncEnabled()
	{
		return m_bVSync;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::SetVSyncEnabled
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::SetVSyncEnabled( bool inEnabled )
	{
		m_bVSync = inEnabled;
		Console::WriteLine( "Engine: VSync has been ", inEnabled ? "enabled" : "disabled" );

		return true;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::SetAlphaBlendingState
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::SetAlphaBlendingState( AlphaBlendingState inState )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null" );

		if( inState != m_AlphaBlendState )
		{
			const float blendFactor[ 4 ] = { 0.f, 0.f, 0.f, 0.f };
			const UINT sampleMask = 0xffffffff;

			m_Context->OMSetBlendState( inState == AlphaBlendingState::Enabled ? m_BlendEnabledState.Get() : m_BlendDisabledState.Get(), blendFactor, sampleMask );
			m_AlphaBlendState = inState;
		}
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::GetAlphaBlendingState
	--------------------------------------------------------------------------------------------*/
	AlphaBlendingState DX11Graphics::GetAlphaBlendingState()
	{
		return m_AlphaBlendState;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::SetDepthStencilState
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::SetDepthStencilState( DepthStencilState inState )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null" );

		if( inState != m_DepthState )
		{
			ID3D11DepthStencilState* targetState = nullptr;
			switch( inState )
			{
				case DepthStencilState::DepthAndStencilEnabled:
					targetState = m_DepthEnabledState.Get();
					break;

				case DepthStencilState::DepthDisabledStencilEnabled:
					targetState = m_DepthDisabledState.Get();
					break;
			}

			if( targetState != nullptr )
			{
				m_Context->OMSetDepthStencilState( targetState, 1 );
			}

			m_DepthState = inState;
		}
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::GetDepthStencilState
	--------------------------------------------------------------------------------------------*/
	DepthStencilState DX11Graphics::GetDepthStencilState()
	{
		return m_DepthState;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::GetBackBuffer
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr< RTexture2D > DX11Graphics::GetBackBuffer()
	{
		return m_BackBuffer;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::ClearRenderTarget
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::ClearRenderTarget( const std::shared_ptr< RTexture2D >& inTarget, float inR, float inG, float inB, float inA )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null!" );

		if( inTarget == nullptr || !inTarget->IsValid() || !inTarget->IsRenderTarget() )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to clear render target, the texture was invalid or didnt have an attached render target" );
			return false;
		}

		auto* texPtr = dynamic_cast< DX11Texture2D* >( inTarget.get() );
		auto* rtvPtr = texPtr ? texPtr->GetRTV() : nullptr;

		HYPERION_VERIFY( rtvPtr != nullptr, "[DX11] Render target view pointer was null, when it shouldnt have been" );

		FLOAT color[] = { inR, inG, inB, inA };
		m_Context->ClearRenderTargetView( rtvPtr, color );
		return true;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::AttachRenderTargets
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::AttachRenderTargets( const std::vector< std::shared_ptr< RTexture2D > >& inTargets, bool bUseDepthStencil )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null!" );

		std::vector< ID3D11RenderTargetView* > rtvList;

		for( auto it = inTargets.begin(); it != inTargets.end(); it++ )
		{
			auto* dx11Tex = dynamic_cast< DX11Texture2D* >( it->get() );
			auto* rtvPtr = dx11Tex ? dx11Tex->GetRTV() : nullptr;

			if( rtvPtr == nullptr )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to attach render targets, one of the targets were invalid or didnt have an attached render target" );
				return false;
			}

			rtvList.push_back( rtvPtr );
		}

		if( rtvList.size() == 0 )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to attach render targets, none were provided!" );
			return false;
		}

		m_Context->OMSetRenderTargets( (UINT) rtvList.size(), rtvList.data(), bUseDepthStencil ? m_DepthStencilView.Get() : nullptr );
		m_AttachedRenderTargets = inTargets;

		return true;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::AttachRenderTarget
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::AttachRenderTarget( const std::shared_ptr< RTexture2D >& inTarget, bool bUseDepthStencil )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null!" );

		auto* texPtr = dynamic_cast< DX11Texture2D* >( inTarget.get() );
		auto rtvPtr = texPtr ? texPtr->GetRTV() : nullptr;

		if( rtvPtr == nullptr )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to attach render target, it was either invalid, or didnt have a render target attached" );
			return false;
		}

		ID3D11RenderTargetView* rtvList[] = { rtvPtr };
		m_Context->OMSetRenderTargets( 1, rtvList, bUseDepthStencil ? m_DepthStencilView.Get() : nullptr );
		m_AttachedRenderTargets = { inTarget };

		return true;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::DetachRenderTargets
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::DetachRenderTargets()
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null!" );

		std::vector< ID3D11RenderTargetView* > nullRtvList;
		nullRtvList.resize( Math::Max( 1U, (uint32) m_AttachedRenderTargets.size() ), nullptr );
		
		m_Context->OMSetRenderTargets( (UINT) nullRtvList.size(), nullRtvList.data(), nullptr );
		m_AttachedRenderTargets.clear();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::GetFloorMesh
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::GetFloorMesh( std::shared_ptr< RBuffer >& outVertexBuffer, std::shared_ptr< RBuffer >& outIndexBuffer, std::vector< Matrix >& outMatricies )
	{
		// Output the buffers we have
		outVertexBuffer		= m_FloorVertexBuffer;
		outIndexBuffer		= m_FloorIndexBuffer;
		outMatricies		= m_FloorMatricies;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::RebuildScreenQuad
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::RebuildScreenQuad( uint32 inWidth, uint32 inHeight )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null!" );
		HYPERION_VERIFY( inWidth > 300 && inHeight > 300, "[DX11] Resolution was invalid" ); // TODO: Replace with actual min width and height

		WindowVertex verts[ 6 ] {};

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

		if( FAILED( m_Device->CreateBuffer( &vertDesc, &vertexData, m_ScreenQuadVertexBuffer.GetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create screen quad vertex buffer!" );
			return;
		}
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::RenderScreenQuad
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::RenderScreenQuad()
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null!" );
		HYPERION_VERIFY( m_ScreenQuadVertexBuffer != nullptr, "[DX11] Screen quad buffer was null!" );

		ID3D11Buffer* vertexBufferList[] = { m_ScreenQuadVertexBuffer.Get() };
		UINT strideList[] = { sizeof( WindowVertex ) };
		UINT offsetList[] = { 0 };

		m_Context->IASetVertexBuffers( 0, 1, vertexBufferList, strideList, offsetList );
		m_Context->Draw( 6, 0 );
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::AttachMesh
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::AttachMesh( const std::shared_ptr<RBuffer>& inVertexBuffer, const std::shared_ptr<RBuffer>& inIndexBuffer, uint32 inIndexCount )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null!" );

		// We need to get the buffer pointers, through some casting
		auto* vertBufferPtr		= dynamic_cast< DX11Buffer* >( inVertexBuffer.get() );
		auto* indxBufferPtr		= dynamic_cast< DX11Buffer* >( inIndexBuffer.get() );
		auto* vertResource		= vertBufferPtr ? vertBufferPtr->Get() : nullptr;
		auto* indxResource		= indxBufferPtr ? indxBufferPtr->Get() : nullptr;

		if( vertResource == nullptr || indxResource == nullptr )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to upload mesh to input assembler, either the vertex or index buffer was null/inavlid" );
			return;
		}
		else if( vertBufferPtr->GetType() != BufferType::Vertex || indxBufferPtr->GetType() != BufferType::Index )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to upload mesh to input assembler, the buffer types were not correct" );
			return;
		}

		// TODO: Allow different vertex types?
		ID3D11Buffer* vertBufferList[]	= { vertResource };
		UINT strideList[]				= { sizeof( Vertex3D ) };
		UINT offsetList[]				= { 0 };

		m_Context->IASetVertexBuffers( 0, 1, vertBufferList, strideList, offsetList );
		m_Context->IASetIndexBuffer( indxResource, DXGI_FORMAT_R32_UINT, 0 );

		// We are going to hold refs to the buffers
		m_AttachedVertexBuffer	= inVertexBuffer;
		m_AttachedIndexBuffer	= inIndexBuffer;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::DetachMesh
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::DetachMesh()
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null!" );

		ID3D11Buffer* vertBuffers[] = { nullptr };
		UINT strideList[] = { 0 };
		UINT offsetList[] = { 0 };

		m_Context->IASetVertexBuffers( 0, 1, vertBuffers, strideList, offsetList );
		m_Context->IASetIndexBuffer( nullptr, DXGI_FORMAT_R32_UINT, 0 );

		m_AttachedVertexBuffer.reset();
		m_AttachedIndexBuffer.reset();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::Render
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::Render( uint32 inInstanceCount )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null!" );

		auto indexCount = m_AttachedIndexBuffer ? m_AttachedIndexBuffer->GetCount() : 0;

		if( inInstanceCount == 0 || indexCount < 3 )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to render, instance and/or index count was invalid" );
			return;
		}

		m_Context->DrawIndexedInstanced( indexCount, inInstanceCount, 0, 0, 0 );
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::DisplayFrame
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::DisplayFrame()
	{
		HYPERION_VERIFY( m_SwapChain != nullptr, "[DX11] Swap Chain was null" );
		m_SwapChain->Present( m_bVSync ? 1 : 0, 0 );
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::IsAsyncResourceCreationAllowed
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::IsAsyncResourceCreationAllowed() const
	{
		return m_bAsyncResourceCreation;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateTexture1D
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RTexture1D> DX11Graphics::CreateTexture1D( const TextureParameters& inParams )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null" );

		// Perform validation on the texture parameters
		if( inParams.Width == 0 || ( !inParams.bAutogenMips && inParams.MipCount == 0 ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 1D Texture, parameters invalid (A dimension is 0)" );
			return nullptr;
		}

		if( inParams.Format == TextureFormat::NONE )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 1D Texture, invalid texture format" );
			return nullptr;
		}

		auto newTexture = std::shared_ptr< DX11Texture1D >( new DX11Texture1D() );
		newTexture->m_Asset = inParams.AssetIdentifier;

		D3D11_TEXTURE1D_DESC desc {};

		desc.Usage = inParams.bDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		desc.Format = TextureFormatToDXGIFormat( inParams.Format );

		bool bCreateUAV = false;
		bool bCreateSRV = false;
		bool bCreateRTV = false;
		
		if( HYPERION_HAS_FLAG( inParams.ResourceAccess, RENDERER_RESOURCE_ACCESS_COMPUTE_WRITE ) )
		{
			desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
			bCreateUAV = true;
		}

		if( HYPERION_HAS_FLAG( inParams.ResourceAccess, RENDERER_RESOURCE_ACCESS_SHADER_RESOURCE ) )
		{
			desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
			bCreateSRV = true;
		}

		if( HYPERION_HAS_FLAG( inParams.ResourceAccess, RENDERER_RESOURCE_ACCESS_RENDER_TARGET ) )
		{
			desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			bCreateRTV = true;
		}

		desc.Width				= inParams.Width;
		desc.ArraySize			= 1;
		desc.MipLevels			= inParams.bAutogenMips ? 0 : inParams.MipCount;
		desc.CPUAccessFlags		= inParams.bDynamic ? D3D11_CPU_ACCESS_WRITE : 0;

		if( inParams.bCPURead )
		{
			desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
		}

		std::vector< D3D11_SUBRESOURCE_DATA > data;
		data.resize( inParams.Data.size() );

		for( int i = 0; i < inParams.Data.size(); i++ )
		{
			data[ i ].pSysMem = inParams.Data[ i ].Data;
			data[ i ].SysMemPitch = inParams.Data[ i ].RowSize;
			data[ i ].SysMemSlicePitch = inParams.Data[ i ].LayerSize;
		}

		if( FAILED( m_Device->CreateTexture1D( &desc, data.data(), newTexture->m_Texture.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 1D Texture, couldnt create the texture itself" );
			newTexture->Shutdown();
			return nullptr;
		}

		if( bCreateUAV )
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc {};

			uavDesc.Format				= TextureFormatToDXGIFormat( inParams.Format );
			uavDesc.ViewDimension		= D3D11_UAV_DIMENSION_TEXTURE1D;
			uavDesc.Texture1D.MipSlice	= 0;

			if( FAILED( m_Device->CreateUnorderedAccessView( newTexture->m_Texture.Get(), &uavDesc, newTexture->m_UAV.ReleaseAndGetAddressOf() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create 1D Texture, couldnt create UAV" );
				newTexture->Shutdown();
				return nullptr;
			}
		}

		if( bCreateSRV )
		{
			// Query data about the texture to see how many mips were creating if using auto mip gen
			UINT mipCount = Math::Max( 1U, inParams.MipCount );

			if( inParams.bAutogenMips )
			{
				D3D11_TEXTURE1D_DESC newDesc {};
				newTexture->m_Texture->GetDesc( &newDesc );

				mipCount = newDesc.MipLevels;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc {};

			srvDesc.Format						= TextureFormatToDXGIFormat( inParams.Format );
			srvDesc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE1D;
			srvDesc.Texture1D.MipLevels			= mipCount;
			srvDesc.Texture1D.MostDetailedMip	= 0;

			if( FAILED( m_Device->CreateShaderResourceView( newTexture->m_Texture.Get(), &srvDesc, newTexture->m_SRV.ReleaseAndGetAddressOf() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create 1D Texture, couldnt create SRV" );
				newTexture->Shutdown();
				return nullptr;
			}
		}

		if( bCreateRTV )
		{
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc {};

			rtvDesc.Format				= TextureFormatToDXGIFormat( inParams.Format );
			rtvDesc.ViewDimension		= D3D11_RTV_DIMENSION_TEXTURE1D;
			rtvDesc.Texture1D.MipSlice	= 0;

			if( FAILED( m_Device->CreateRenderTargetView( newTexture->m_Texture.Get(), &rtvDesc, newTexture->m_RTV.ReleaseAndGetAddressOf() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create 1D Texture, couldnt create RTV" );
				newTexture->Shutdown();
				return nullptr;
			}
		}

		return newTexture;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateTexture2D
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RTexture2D> DX11Graphics::CreateTexture2D( const TextureParameters& inParams )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null" );

		// Perform validation on the texture parameters
		if( inParams.Width == 0 || inParams.Height == 0 || ( !inParams.bAutogenMips && inParams.MipCount == 0 ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 2D Texture, parameters invalid (A dimension is 0)" );
			return nullptr;
		}

		if( inParams.Format == TextureFormat::NONE )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 2D Texture, invalid texture format" );
			return nullptr;
		}

		auto newTexture = std::shared_ptr< DX11Texture2D >( new DX11Texture2D() );
		newTexture->m_Asset = inParams.AssetIdentifier;

		D3D11_TEXTURE2D_DESC desc {};

		desc.Usage = inParams.bDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		desc.Format = TextureFormatToDXGIFormat( inParams.Format );

		bool bCreateUAV = false;
		bool bCreateSRV = false;
		bool bCreateRTV = false;

		if( HYPERION_HAS_FLAG( inParams.ResourceAccess, RENDERER_RESOURCE_ACCESS_COMPUTE_WRITE ) )
		{
			desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
			bCreateUAV = true;
		}

		if( HYPERION_HAS_FLAG( inParams.ResourceAccess, RENDERER_RESOURCE_ACCESS_SHADER_RESOURCE ) )
		{
			desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
			bCreateSRV = true;
		}

		if( HYPERION_HAS_FLAG( inParams.ResourceAccess, RENDERER_RESOURCE_ACCESS_RENDER_TARGET ) )
		{
			desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			bCreateRTV = true;
		}

		desc.Width				= inParams.Width;
		desc.Height				= inParams.Height;
		desc.ArraySize			= 1;
		desc.MipLevels			= inParams.bAutogenMips ? 0 : inParams.MipCount;
		desc.CPUAccessFlags		= inParams.bDynamic ? D3D11_CPU_ACCESS_WRITE : 0;

		if( inParams.bCPURead )
		{
			desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
		}

		std::vector< D3D11_SUBRESOURCE_DATA > data;
		data.resize( inParams.Data.size() );

		for( int i = 0; i < inParams.Data.size(); i++ )
		{
			data[ i ].pSysMem = inParams.Data[ i ].Data;
			data[ i ].SysMemPitch = inParams.Data[ i ].RowSize;
			data[ i ].SysMemSlicePitch = inParams.Data[ i ].LayerSize;
		}

		if( FAILED( m_Device->CreateTexture2D( &desc, data.data(), newTexture->m_Texture.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 2D Texture, couldnt create the texture itself" );
			newTexture->Shutdown();
			return nullptr;
		}

		if( bCreateUAV )
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc {};

			uavDesc.Format = TextureFormatToDXGIFormat( inParams.Format );
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture1D.MipSlice = 0;

			if( FAILED( m_Device->CreateUnorderedAccessView( newTexture->m_Texture.Get(), &uavDesc, newTexture->m_UAV.ReleaseAndGetAddressOf() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create 1D Texture, couldnt create UAV" );
				newTexture->Shutdown();
				return nullptr;
			}
		}

		if( bCreateSRV )
		{
			// Query data about the texture to see how many mips were creating if using auto mip gen
			UINT mipCount = Math::Max( 1U, inParams.MipCount );

			if( inParams.bAutogenMips )
			{
				D3D11_TEXTURE2D_DESC newDesc {};
				newTexture->m_Texture->GetDesc( &newDesc );

				mipCount = newDesc.MipLevels;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc {};

			srvDesc.Format						= TextureFormatToDXGIFormat( inParams.Format );
			srvDesc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture1D.MipLevels			= mipCount;
			srvDesc.Texture1D.MostDetailedMip	= 0;

			if( FAILED( m_Device->CreateShaderResourceView( newTexture->m_Texture.Get(), &srvDesc, newTexture->m_SRV.ReleaseAndGetAddressOf() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create 2D Texture, couldnt create SRV" );
				newTexture->Shutdown();
				return nullptr;
			}
		}

		if( bCreateRTV )
		{
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc {};

			rtvDesc.Format				= TextureFormatToDXGIFormat( inParams.Format );
			rtvDesc.ViewDimension		= D3D11_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture1D.MipSlice	= 0;

			if( FAILED( m_Device->CreateRenderTargetView( newTexture->m_Texture.Get(), &rtvDesc, newTexture->m_RTV.ReleaseAndGetAddressOf() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create 2D Texture, couldnt create RTV" );
				newTexture->Shutdown();
				return nullptr;
			}
		}

		return newTexture;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateTexture3D
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RTexture3D> DX11Graphics::CreateTexture3D( const TextureParameters& inParams )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null" );

		// Perform validation on the texture parameters
		if( inParams.Width == 0 || inParams.Height == 0 || inParams.Depth == 0 || ( !inParams.bAutogenMips && inParams.MipCount == 0 ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 3D Texture, parameters invalid (A dimension is 0)" );
			return nullptr;
		}

		if( inParams.Format == TextureFormat::NONE )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 3D Texture, invalid texture format" );
			return nullptr;
		}

		auto newTexture			= std::shared_ptr< DX11Texture3D >( new DX11Texture3D() );
		newTexture->m_Asset		= inParams.AssetIdentifier;

		D3D11_TEXTURE3D_DESC desc {};

		desc.Usage		= inParams.bDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		desc.Format		= TextureFormatToDXGIFormat( inParams.Format );

		bool bCreateUAV = false;
		bool bCreateSRV = false;
		bool bCreateRTV = false;

		if( HYPERION_HAS_FLAG( inParams.ResourceAccess, RENDERER_RESOURCE_ACCESS_COMPUTE_WRITE ) )
		{
			desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
			bCreateUAV = true;
		}

		if( HYPERION_HAS_FLAG( inParams.ResourceAccess, RENDERER_RESOURCE_ACCESS_SHADER_RESOURCE ) )
		{
			desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
			bCreateSRV = true;
		}

		if( HYPERION_HAS_FLAG( inParams.ResourceAccess, RENDERER_RESOURCE_ACCESS_RENDER_TARGET ) )
		{
			desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			bCreateRTV = true;
		}

		desc.Width				= inParams.Width;
		desc.Height				= inParams.Height;
		desc.Depth				= inParams.Depth;
		desc.MipLevels			= inParams.bAutogenMips ? 0 : inParams.MipCount;
		desc.CPUAccessFlags		= inParams.bDynamic ? D3D11_CPU_ACCESS_WRITE : 0;

		if( inParams.bCPURead )
		{
			desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
		}

		std::vector< D3D11_SUBRESOURCE_DATA > data;
		data.resize( inParams.Data.size() );

		for( int i = 0; i < inParams.Data.size(); i++ )
		{
			data[ i ].pSysMem			= inParams.Data[ i ].Data;
			data[ i ].SysMemPitch		= inParams.Data[ i ].RowSize;
			data[ i ].SysMemSlicePitch	= inParams.Data[ i ].LayerSize;
		}

		if( FAILED( m_Device->CreateTexture3D( &desc, data.data(), newTexture->m_Texture.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 3D Texture, couldnt create the texture itself" );
			newTexture->Shutdown();
			return nullptr;
		}

		if( bCreateUAV )
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc {};

			uavDesc.Format				= TextureFormatToDXGIFormat( inParams.Format );
			uavDesc.ViewDimension		= D3D11_UAV_DIMENSION_TEXTURE3D;
			uavDesc.Texture1D.MipSlice	= 0;

			if( FAILED( m_Device->CreateUnorderedAccessView( newTexture->m_Texture.Get(), &uavDesc, newTexture->m_UAV.ReleaseAndGetAddressOf() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create 3D Texture, couldnt create UAV" );
				newTexture->Shutdown();
				return nullptr;
			}
		}

		if( bCreateSRV )
		{
			// Query data about the texture to see how many mips were creating if using auto mip gen
			UINT mipCount = Math::Max( 1U, inParams.MipCount );

			if( inParams.bAutogenMips )
			{
				D3D11_TEXTURE3D_DESC newDesc {};
				newTexture->m_Texture->GetDesc( &newDesc );

				mipCount = newDesc.MipLevels;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc {};

			srvDesc.Format						= TextureFormatToDXGIFormat( inParams.Format );
			srvDesc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE3D;
			srvDesc.Texture1D.MipLevels			= mipCount;
			srvDesc.Texture1D.MostDetailedMip	= 0;

			if( FAILED( m_Device->CreateShaderResourceView( newTexture->m_Texture.Get(), &srvDesc, newTexture->m_SRV.ReleaseAndGetAddressOf() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create 3D Texture, couldnt create SRV" );
				newTexture->Shutdown();
				return nullptr;
			}
		}

		if( bCreateRTV )
		{
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc {};

			rtvDesc.Format				= TextureFormatToDXGIFormat( inParams.Format );
			rtvDesc.ViewDimension		= D3D11_RTV_DIMENSION_TEXTURE3D;
			rtvDesc.Texture1D.MipSlice	= 0;

			if( FAILED( m_Device->CreateRenderTargetView( newTexture->m_Texture.Get(), &rtvDesc, newTexture->m_RTV.ReleaseAndGetAddressOf() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create 3D Texture, couldnt create RTV" );
				newTexture->Shutdown();
				return nullptr;
			}
		}

		return newTexture;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateTexture1D
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RTexture1D> DX11Graphics::CreateTexture1D()
	{
		return std::shared_ptr< RTexture1D >( new DX11Texture1D() );
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateTexture2D
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RTexture2D> DX11Graphics::CreateTexture2D()
	{
		return std::shared_ptr< RTexture2D >( new DX11Texture2D() );
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateTexture3D
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RTexture3D> DX11Graphics::CreateTexture3D()
	{
		return std::shared_ptr< RTexture3D >( new DX11Texture3D() );
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateBuffer
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RBuffer> DX11Graphics::CreateBuffer( const BufferParameters& inParams )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null" );
		
		if( inParams.ElementSize % 16 != 0 )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create buffer.. element size should be a multiple of 16 bytes! (Element Size: ", inParams.ElementSize, ")" );
			return nullptr;
		}

		// First, we need to create an empty buffer
		auto newBuffer = std::shared_ptr< DX11Buffer >( new DX11Buffer( inParams.Type ) );

		// Set some buffer attributes
		newBuffer->m_TotalSize			= inParams.ElementSize * inParams.Count;
		newBuffer->m_ElementSize		= inParams.ElementSize;
		newBuffer->m_ElementCount		= inParams.Count;
		newBuffer->m_AssetIdentifier	= inParams.SourceAsset;

		// Now lets create the DX buffer resource
		D3D11_BUFFER_DESC desc {};

		desc.Usage = inParams.Dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
		
		switch( inParams.Type )
		{
		case BufferType::Vertex:
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			break;
		case BufferType::Index:
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			break;
		case BufferType::Constants:
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			break;
		case BufferType::Structured:
			desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			break;
		case BufferType::StructuredAppend:
			desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			break;
		}

		bool bCreateUAV = false;
		bool bCreateSRV = false;

		if( HYPERION_HAS_FLAG( inParams.ResourceAccess, RENDERER_RESOURCE_ACCESS_COMPUTE_WRITE ) )
		{
			desc.BindFlags	= D3D11_BIND_UNORDERED_ACCESS;
			bCreateUAV		= true;
		}

		if( HYPERION_HAS_FLAG( inParams.ResourceAccess, RENDERER_RESOURCE_ACCESS_SHADER_RESOURCE ) )
		{
			desc.BindFlags	= D3D11_BIND_SHADER_RESOURCE;
			bCreateSRV		= true;
		}

		if( inParams.CanCPURead )
		{
			desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
		}

		if( inParams.Dynamic )
		{
			desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
		}

		desc.ByteWidth				= newBuffer->m_TotalSize;
		desc.StructureByteStride	= newBuffer->m_ElementSize;

		// Create the actual buffer
		D3D11_SUBRESOURCE_DATA data {};

		data.pSysMem = inParams.Data;
		
		if( FAILED( m_Device->CreateBuffer( &desc, &data, newBuffer->m_Buffer.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create buffer resource! Couldnt create the actual buffer" );
			return nullptr;
		}

		// Create any needed views of the buffer resource
		if( bCreateSRV )
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc {};

			srvDesc.Format					= DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension			= D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.ElementOffset	= 0;
			srvDesc.Buffer.ElementWidth		= newBuffer->m_ElementSize;
			srvDesc.Buffer.FirstElement		= 0;
			srvDesc.Buffer.NumElements		= newBuffer->m_ElementCount;

			if( FAILED( m_Device->CreateShaderResourceView( newBuffer->m_Buffer.Get(), &srvDesc, newBuffer->m_SRV.ReleaseAndGetAddressOf() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create buffer resource! Couldnt create the shader resource view" );
				newBuffer->Shutdown();
				return nullptr;
			}
		}

		if( bCreateUAV )
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc {};

			uavDesc.Format					= DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension			= D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement		= 0;
			uavDesc.Buffer.Flags			= inParams.Type == BufferType::StructuredAppend ? D3D11_BUFFER_UAV_FLAG_APPEND : 0;
			uavDesc.Buffer.NumElements		= newBuffer->m_ElementCount;

			if( FAILED( m_Device->CreateUnorderedAccessView( newBuffer->m_Buffer.Get(), &uavDesc, newBuffer->m_UAV.ReleaseAndGetAddressOf() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create buffer resource! Couldnt create the unordered access view" );
				newBuffer->Shutdown();
				return nullptr;
			}
		}

		return newBuffer;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateBuffer
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RBuffer> DX11Graphics::CreateBuffer( BufferType inType )
	{
		// We are just creating an empty buffer, so we can actually insert the ID3D11Buffer into it at a later point
		return std::shared_ptr< RBuffer >( new DX11Buffer( inType ) );
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CopyResource
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::CopyResource( const std::shared_ptr< RGraphicsResource >& inSource, const std::shared_ptr< RGraphicsResource >& inTarget )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null!" );

		// We need to make sure that the two resource are the same type, otherwise the API operation will fail
		auto* sourcePtr = dynamic_cast< RDX11Resource* >( inSource.get() );
		auto* targetPtr = dynamic_cast< RDX11Resource* >( inTarget.get() );
		auto* dx11SrcPtr = sourcePtr ? sourcePtr->GetResource() : nullptr;
		auto* dx11TrgPtr = targetPtr ? targetPtr->GetResource() : nullptr;

		if( dx11SrcPtr == nullptr || dx11TrgPtr == nullptr )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to copy resource, either source or destination was null" );
			return false;
		}

		D3D11_RESOURCE_DIMENSION sourceDimension;
		D3D11_RESOURCE_DIMENSION targetDimension;

		dx11SrcPtr->GetType( &sourceDimension );
		dx11TrgPtr->GetType( &targetDimension );

		if( sourceDimension != targetDimension )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to copy resource, the source and target resources were not the same type" );
			return false;
		}

		m_Context->CopyResource( dx11TrgPtr, dx11SrcPtr );
		return true;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CopyResourceRange
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::CopyResourceRange( const std::shared_ptr< RGraphicsResource >& inSource, const std::shared_ptr< RGraphicsResource >& inTarget, const ResourceRangeParameters& inParams )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null!" );

		// We need to make sure the two resources are the same type
		auto* sourcePtr = dynamic_cast<RDX11Resource*>( inSource.get() );
		auto* targetPtr = dynamic_cast<RDX11Resource*>( inTarget.get() );
		auto* dx11SrcPtr = sourcePtr ? sourcePtr->GetResource() : nullptr;
		auto* dx11TrgPtr = targetPtr ? targetPtr->GetResource() : nullptr;

		if( dx11SrcPtr == nullptr || dx11TrgPtr == nullptr )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to copy resource range, either the source or destination was null" );
			return false;
		}

		D3D11_RESOURCE_DIMENSION sourceDimension;
		D3D11_RESOURCE_DIMENSION targetDimension;

		dx11SrcPtr->GetType( &sourceDimension );
		dx11TrgPtr->GetType( &targetDimension );

		if( sourceDimension != targetDimension )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to copy resource range, the source and target resource were not the same type!" );
			return false;
		}
		
		UINT srcSubresourceIndex	= inParams.sourceMip;
		UINT destSubresourceIndex	= inParams.destMip;
		UINT destX					= inParams.destX;
		UINT destY					= inParams.destY;
		UINT destZ					= inParams.destZ;
		UINT copyFlags				= 0;

		D3D11_BOX regionSize {};

		regionSize.left		= inParams.sourceX;
		regionSize.top		= inParams.sourceY;
		regionSize.front	= inParams.sourceZ;
		regionSize.right	= regionSize.left + inParams.rangeWidth;
		regionSize.bottom	= regionSize.top + inParams.rangeHeight;
		regionSize.back		= regionSize.front + inParams.rangeDepth;
	
		m_Context->CopySubresourceRegion( dx11TrgPtr, destSubresourceIndex, destX, destY, destZ, dx11SrcPtr, srcSubresourceIndex, &regionSize );
		return true;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateVertexShader
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RVertexShader> DX11Graphics::CreateVertexShader( VertexShaderType inType )
	{
		return std::shared_ptr<RVertexShader>();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateGeometryShader
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RGeometryShader> DX11Graphics::CreateGeometryShader( GeometryShaderType inType )
	{
		return std::shared_ptr<RGeometryShader>();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreatePixelShader
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RPixelShader> DX11Graphics::CreatePixelShader( PixelShaderType inType )
	{
		return std::shared_ptr<RPixelShader>();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateComputeShader
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RComputeShader> DX11Graphics::CreateComputeShader( ComputeShaderType inType )
	{
		return std::shared_ptr<RComputeShader>();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreatePostProcessShader
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RPostProcessShader> DX11Graphics::CreatePostProcessShader( PostProcessShaderType inType )
	{
		return std::shared_ptr<RPostProcessShader>();
	}
}