/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/DX11Graphics.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DirectX11/DX11Graphics.h"

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
		m_GraphicsDeviceName(), m_DedicatedVideoMemory( 0 ), m_SharedVideoMemory( 0 )
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
		uint32 dedicatedGraphicsMemory		= 0;
		uint32 sharedGraphicsMemory			= 0;

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

			// Ensure the window size matches the resolution were targetting
			HWND windowHandle = static_cast< HWND >( inParameters.ptrOSWindow );
			ResizeWindow( windowHandle, targetResolution );

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
			dedicatedGraphicsMemory		= gpuDesc.DedicatedVideoMemory;
			sharedGraphicsMemory		= gpuDesc.SharedSystemMemory;

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
			swapChainDesc.BufferDesc.RefreshRate.Numerator		= 0;
			swapChainDesc.BufferDesc.RefreshRate.Denominator	= inParameters.bEnableVSync ? 1 : 0;

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

			// Create our back buffer render target view
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc {};

			rtvDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			rtvDesc.Texture2D.MipSlice	= 0;

			if( FAILED( m_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**) m_BackBuffer.ReleaseAndGetAddressOf() ) ) ||
				FAILED( m_Device->CreateRenderTargetView( m_BackBuffer.Get(), &rtvDesc, m_BackBufferRenderTarget.ReleaseAndGetAddressOf() ) ) )
			{
				throw std::exception( "failed to create back buffer render target" );
			}

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

		return true;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::Shutdown
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::Shutdown()
	{
		m_BlendDisabledState.Reset();
		m_BlendEnabledState.Reset();
		m_RasterState.Reset();
		m_DepthStencilView.Reset();
		m_DepthBuffer.Reset();
		m_DepthEnabledState.Reset();
		m_DepthDisabledState.Reset();
		m_BackBufferRenderTarget.Reset();
		m_BackBuffer.Reset();
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

		m_GraphicsDeviceName.Clear();
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
		ID3D11RenderTargetView* views[] = { nullptr, nullptr, nullptr };
		m_Context->OMSetRenderTargets( 3, views, nullptr );

		// Release resources that we need to re-create
		m_BackBufferRenderTarget.Reset();
		m_BackBuffer.Reset();
		m_DepthStencilView.Reset();
		m_DepthBuffer.Reset();

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

		if( FAILED( m_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**) m_BackBuffer.ReleaseAndGetAddressOf() ) ) ||
			FAILED( m_Device->CreateRenderTargetView( m_BackBuffer.Get(), &rtvDesc, m_BackBufferRenderTarget.ReleaseAndGetAddressOf() ) ) ||
			FAILED( m_Device->CreateTexture2D( &bufferDesc, NULL, m_DepthBuffer.ReleaseAndGetAddressOf() ) ) ||
			FAILED( m_Device->CreateDepthStencilView( m_DepthBuffer.Get(), &dsvDesc, m_DepthStencilView.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[FATAL] DX11: Failed to resize main resources" );
			MessageBox( m_OSTarget, L"Hyperion Engine Fatal Error!", L"Failed to resize engine resources!", MB_OK );
			std::terminate();
			return false;
		}

		D3D11_VIEWPORT viewport {};

		viewport.Width		= selectedWidth;
		viewport.Height		= selectedHeight;
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

			for( auto it = modeList.begin(); it != modeList.end(); it++ )
			{
				ScreenResolution res {};

				res.Width = it->Width;
				res.Height = it->Height;
				res.FullScreen = false;

				output.emplace_back( res );
			}
		}

		outResolutions = output;
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
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Immediate context was null" );

		// TODO:
		// Do we have to update the swap chain? When we create it, we specify a refresh rate
		// Or.. is this refresh rate just to let the API know what VSync refresh rate should be for the monitor?
		// Or.. is it needed to properly enable VSync?


	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::SetAlphaBlendingState
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::SetAlphaBlendingState( AlphaBlendingState inState )
	{

	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::GetAlphaBlendingState
	--------------------------------------------------------------------------------------------*/
	AlphaBlendingState DX11Graphics::GetAlphaBlendingState()
	{
		return AlphaBlendingState();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::SetDepthStencilState
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::SetDepthStencilState( DepthStencilState inState )
	{

	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::GetDepthStencilState
	--------------------------------------------------------------------------------------------*/
	DepthStencilState DX11Graphics::GetDepthStencilState()
	{
		return DepthStencilState();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::GetBackBufferRenderTarget
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RRenderTarget> DX11Graphics::GetBackBufferRenderTarget()
	{
		return std::shared_ptr<RRenderTarget>();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateRenderTarget
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RRenderTarget> DX11Graphics::CreateRenderTarget( const std::shared_ptr<RTexture2D>& inTexture )
	{
		return std::shared_ptr<RRenderTarget>();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::ClearRenderTarget
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::ClearRenderTarget( const std::shared_ptr<RRenderTarget>& inTarget, float inR, float inG, float inB, float inA )
	{
		return false;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::AttachRenderTargets
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::AttachRenderTargets( const std::vector<std::shared_ptr<RRenderTarget>>& inTargets, bool bUseDepthStencil )
	{

	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::AttachRenderTarget
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::AttachRenderTarget( const std::shared_ptr<RRenderTarget>& inTarget, bool bUseDepthStencil )
	{

	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::DetachRenderTargets
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::DetachRenderTargets()
	{

	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::GetFloorMesh
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::GetFloorMesh( std::shared_ptr<RBuffer>& outVertexBuffer, std::shared_ptr<RBuffer>& outIndexBuffer, std::vector<Matrix>& outMatricies )
	{

	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::RenderScreenQuad
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::RenderScreenQuad()
	{

	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::UploadMesh
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::UploadMesh( const std::shared_ptr<RBuffer>& inVertexBuffer, const std::shared_ptr<RBuffer>& inIndexBuffer, uint32 inIndexCount )
	{

	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::Render
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::Render( uint32 inInstanceCount )
	{

	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::DisplayFrame
	--------------------------------------------------------------------------------------------*/
	void DX11Graphics::DisplayFrame()
	{

	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::IsAsyncResourceCreationAllowed
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::IsAsyncResourceCreationAllowed() const
	{
		return false;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateTexture1D
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RTexture1D> DX11Graphics::CreateTexture1D( const TextureParameters& inParams )
	{
		return std::shared_ptr<RTexture1D>();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateTexture2D
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RTexture2D> DX11Graphics::CreateTexture2D( const TextureParameters& inParams )
	{
		return std::shared_ptr<RTexture2D>();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateTexture3D
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RTexture3D> DX11Graphics::CreateTexture3D( const TextureParameters& inParams )
	{
		return std::shared_ptr<RTexture3D>();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateTexture1D
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RTexture1D> DX11Graphics::CreateTexture1D()
	{
		return std::shared_ptr<RTexture1D>();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateTexture2D
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RTexture2D> DX11Graphics::CreateTexture2D()
	{
		return std::shared_ptr<RTexture2D>();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateTexture3D
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RTexture3D> DX11Graphics::CreateTexture3D()
	{
		return std::shared_ptr<RTexture3D>();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateBuffer
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RBuffer> DX11Graphics::CreateBuffer( const BufferParameters& inParams )
	{
		return std::shared_ptr<RBuffer>();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CreateBuffer
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr<RBuffer> DX11Graphics::CreateBuffer( BufferType inType )
	{
		return std::shared_ptr<RBuffer>();
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CopyResource
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::CopyResource( const std::shared_ptr<RTexture1D>& inSource, const std::shared_ptr<RTexture1D>& inTarget )
	{
		return false;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CopyResource
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::CopyResource( const std::shared_ptr< RTexture2D >& inSource, const std::shared_ptr< RTexture2D >& inTarget )
	{
		return false;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CopyResource
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::CopyResource( const std::shared_ptr< RTexture3D >& inSource, const std::shared_ptr< RTexture3D >& inTarget )
	{
		return false;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CopyResource
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::CopyResource( const std::shared_ptr< RBuffer >& inSource, const std::shared_ptr< RBuffer >& inTarget )
	{
		return false;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CopyResourceRange
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::CopyResourceRange( const std::shared_ptr<RTexture1D>& inSource, const std::shared_ptr<RTexture1D>& inTarget, const ResourceRangeParameters& inParams )
	{
		return false;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CopyResourceRange
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::CopyResourceRange( const std::shared_ptr< RTexture2D >& inSource, const std::shared_ptr< RTexture2D >& inTarget, const ResourceRangeParameters& inParams )
	{
		return false;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CopyResourceRange
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::CopyResourceRange( const std::shared_ptr< RTexture3D >& inSource, const std::shared_ptr< RTexture3D >& inTarget, const ResourceRangeParameters& inParams )
	{
		return false;
	}

	/*--------------------------------------------------------------------------------------------
		DX11Graphics::CopyResourceRange
	--------------------------------------------------------------------------------------------*/
	bool DX11Graphics::CopyResourceRange( const std::shared_ptr< RBuffer >& inSource, const std::shared_ptr< RBuffer >& inTarget, const ResourceRangeParameters& inParams )
	{
		return false;
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



	//////////////////////////////////////// OLD //////////////////////////////////////////////////
	/*------------------------------------------------------------------------------------------
		DirectX11Graphics Constructor 
	------------------------------------------------------------------------------------------*/
	DirectX11Graphics::DirectX11Graphics()
		: m_bRunning( false ), m_bVSync( false ), m_Resolution(), m_bDepthEnabled( true ), m_AttachedGeometryAssetIdentifier( ASSET_INVALID )
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
				Console::WriteLine( "[ERROR] DX11: Failed to get reoslution list.. couldnt get output device" );
				return false;
			}

			// Get list of supported display modes on this output device
			uint32 numberModes = 0;
			if( FAILED( output->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numberModes, NULL ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to get reoslution list.. couldnt get display modes list" );
				output->Release();
				return false;
			}

			// Create array of supported modes
			DXGI_MODE_DESC* supportedModes = new DXGI_MODE_DESC[ numberModes ];
			ZeroMemory( supportedModes, sizeof( DXGI_MODE_DESC ) * numberModes );

			// Fill out this array
			if( FAILED( output->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numberModes, supportedModes ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to get resolution list.. couldnt fill display modes list!" );
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
				Console::WriteLine( "[ERROR] DX11: Failed to update resolution.. selected resolution of ", inResolution.Width, "x", inResolution.Height, " is invalid!" );
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
						Console::WriteLine( "[ERROR] DX11: Failed to update reoslution.. couldnt resize swap chain target" );
						return false;
					}

					// Set swap chain to fullscreen
					if( FAILED( m_SwapChain->SetFullscreenState( TRUE, NULL ) ) )
					{
						Console::WriteLine( "[ERROR] DX11: Failed to update reoslution.. couldnt set fullscreen state" );
						return false;
					}
				}
				else
				{
					// Disable fullscreen
					if( FAILED( m_SwapChain->SetFullscreenState( FALSE, NULL ) ) )
					{
						Console::WriteLine( "[ERROR] DX11: Failed to update resolution.. couldnt exit fullscreen mode" );
						return false;
					}

					// Set window size to our resolution
					RECT newSize = { 0, 0, (long) targetMode.Width, (long) targetMode.Height };
					if( !AdjustWindowRectEx( &newSize, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW ) )
					{
						Console::WriteLine( "[ERROR] DX11: Failed to update resolution.. couldnt resize window when exiting fullscreen" );
						return false;
					}

					// Center the window
					SetWindowPos( m_Output, HWND_TOP, 0, 0, newSize.right - newSize.left, newSize.bottom - newSize.top, SWP_NOMOVE );
				}
			}

			// Resize the target
			if( FAILED( m_SwapChain->ResizeTarget( &targetMode ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to update reoslution.. couldnt resize swap chain target" );
				return false;
			}

			// Reset the render target view and depth stencil view.. we need to recreate with updated sizes
			m_RenderTarget.reset();
			m_BackBuffer.reset();

			// Resize the swap chain buffers
			if( FAILED( m_SwapChain->ResizeBuffers( 0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to update resolution.. couldnt update swap chain buffer size" );
				return false;
			}

			if( FAILED( m_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)m_BackBuffer->GetAddress() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to update resolution.. coudlnt recreate back buffer" );
				return false;
			}

			if( FAILED( m_Device->CreateRenderTargetView( m_BackBuffer->Get(), NULL, m_RenderTarget->GetAddress() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to update resolution.. couldnt create render target view" );
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
			viewport.TopLeftY	= 0.f;

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
				Console::WriteLine( "[ERROR] DX11: Failed to get reoslution list.. couldnt get output device" );
				return outputList;
			}

			// Get list of supported display modes on this output device
			uint32 numberModes = 0;
			if( FAILED( output->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 0, &numberModes, NULL ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to get reoslution list.. couldnt get display modes list" );
				output->Release();
				return outputList;
			}

			// Create array of supported modes
			DXGI_MODE_DESC* supportedModes = new DXGI_MODE_DESC[ numberModes ];
			ZeroMemory( supportedModes, sizeof( DXGI_MODE_DESC ) * numberModes );

			// Fill out this array
			if( FAILED( output->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 0, &numberModes, supportedModes ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to get resolution list.. couldnt fill display modes list!" );
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
		HYPERION_VERIFY( !m_Device && !m_DeviceContext, "[DX11] Attempt to init graphics api, but it is already initialized" );

		Console::WriteLine( "[STATUS] DX11: Initializing..." );

		if( !pWindow )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to initialize.. output window invalid!" );
			return false;
		}

		HWND windowHandle = static_cast< HWND >( pWindow );

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
		defaultView.Rotation	= Quaternion();
		defaultView.FOV			= DirectX::XM_PIDIV4;

		SetCameraInfo( defaultView );

		// Set primitive topology
		m_DeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		m_bRunning = true;
		return true;
	}


	/*------------------------------------------------------------------------------------------
		DirectX11Graphics::Shutdown 
	------------------------------------------------------------------------------------------*/
	void DirectX11Graphics::Shutdown()
	{
		if( m_bRunning )
		{
			Console::WriteLine( "DirectX11: Shutting down..." );
		}

		ShutdownResources();
		m_bRunning = false;
	}

	/*------------------------------------------------------------------------------------------
		DirectX11Graphics::InitializeResources 
	------------------------------------------------------------------------------------------*/
	bool DirectX11Graphics::InitializeResources( HWND Target, ScreenResolution& Resolution )
	{
		if( !Target ) return false;

		// Create pointers to dxgi resources we will create in our try block
		IDXGIFactory1* ptrFactory			= nullptr;
		IDXGIAdapter* ptrAdapter			= nullptr;
		IDXGIOutput* ptrOutput				= nullptr;
		DXGI_MODE_DESC* ptrDisplayModes		= nullptr;
		
		try
		{
			// Create graphics interface factory
			HRESULT res = CreateDXGIFactory1( __uuidof( IDXGIFactory1 ), (void**) &ptrFactory );
			if( FAILED( res ) || !ptrFactory )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to initialize resources.. couldnt create graphics interface factory" );
				throw std::exception();
			}

			// Create video device adapter
			res = ptrFactory->EnumAdapters( 0, &ptrAdapter );
			if( FAILED( res ) || !ptrAdapter )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to initialize resources.. couldnt enumerate adapters" );
				throw std::exception();
			}

			res = ptrAdapter->EnumOutputs( 0, &ptrOutput );
			if( FAILED( res ) || !ptrOutput )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to initialize resources.. couldnt enum outputs!" );
				throw std::exception();
			}

			unsigned int numModes = 0;
			res = ptrOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to initialize resources.. couldnt get display modes list" );
				throw std::exception();
			}

			ptrDisplayModes = new DXGI_MODE_DESC[ numModes ];
			res = ptrOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_ENUM_MODES_INTERLACED, &numModes, ptrDisplayModes );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to initialize resources.. couldnt fill display mode list" );
				throw std::exception();
			}

			if( numModes <= 0 )
			{
				Console::WriteLine( "[ERROR] DX11: There are no supported display modes! Failed to initialize" );
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
				Console::WriteLine( "[WARNING] DX11: Selected resolution (", Resolution.Width, "x", Resolution.Height, ") wasnt supported.. defaulting!" );
				selectedMode = ptrDisplayModes[ numModes - 1 ];
				Console::WriteLine( "[WARNING] DX11: Defaulting to resolution (", selectedMode.Width, "x", selectedMode.Height, ")" );
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
				Console::WriteLine( "[ERROR] DX11: Failed to initialize resources.. coudlnt read video card information" );
				throw std::exception();
			}

			m_GraphicsMemory	= (uint32)( videoCardDescription.DedicatedVideoMemory / 1024 / 1024 );
			m_GraphicsDevice	= String( std::wstring( videoCardDescription.Description ), StringEncoding::UTF16 );

			// Resize window to match our target resolution
			RECT newSize = { 0, 0, (long) Resolution.Width, (long) Resolution.Height };
			if( !AdjustWindowRectEx( &newSize, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to resize window during initialization!\n" );
				throw std::exception();
			}

			// Center the window
			SetWindowPos( m_Output, HWND_TOP, 0, 0, newSize.right - newSize.left, newSize.bottom - newSize.top, SWP_NOMOVE );

			// Create swap chain
			DXGI_SWAP_CHAIN_DESC swapChainDesc;
			ZeroMemory( &swapChainDesc, sizeof( swapChainDesc ) );

			swapChainDesc.BufferCount			= 2;
			swapChainDesc.BufferDesc.Width		= selectedMode.Width;
			swapChainDesc.BufferDesc.Height		= selectedMode.Height;
			swapChainDesc.BufferDesc.Format		= DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.OutputWindow			= Target;
			swapChainDesc.SampleDesc.Count		= 1;
			swapChainDesc.SampleDesc.Quality	= 0;
			swapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD; 
			swapChainDesc.Flags					= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			swapChainDesc.Windowed				= !Resolution.FullScreen;

			swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = m_bVSync ? 1 : 0;

			// Create the device and context
			//D3D_FEATURE_LEVEL targetFeatureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
			D3D_FEATURE_LEVEL targetFeatureLevels[] = { D3D_FEATURE_LEVEL_11_1 };
			D3D_FEATURE_LEVEL loadedFeatureLevel;
			UINT deviceFlags = 0;

			#ifdef HYPERION_DEBUG_RENDERER
			deviceFlags = D3D11_CREATE_DEVICE_DEBUG;
			#endif

			res = D3D11CreateDevice( ptrAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, deviceFlags, targetFeatureLevels, 1, D3D11_SDK_VERSION, 
									 m_Device.ReleaseAndGetAddressOf(), &loadedFeatureLevel, m_DeviceContext.ReleaseAndGetAddressOf() );

			if( FAILED( res ) || !m_Device || !m_DeviceContext )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to initialize, device couldnt be created" );
				throw std::exception();
			}

			// Check feature level
			/*
			if( loadedFeatureLevel == D3D_FEATURE_LEVEL_11_0 )
			{
				Console::WriteLine( "DX11: Couldnt load DirectX 11.1, instead using 11.0" );
			}
			else if( loadedFeatureLevel != D3D_FEATURE_LEVEL_11_1 )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to load required feature level!" );
				throw std::exception();
			}
			*/
			if( loadedFeatureLevel != D3D_FEATURE_LEVEL_11_1 )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to load required feature level (11.2)!" );
				throw std::exception();
			}

			// Query for 11.2 device
			ID3D11Device2* newDevice = nullptr;
			if( FAILED( m_Device->QueryInterface< ID3D11Device2 >( &newDevice ) ) || newDevice == nullptr )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to get 11.2 device!" );
				throw std::exception();
			}

			// Now, lets create the swap chain
			res = ptrFactory->CreateSwapChain( m_Device.Get(), &swapChainDesc, m_SwapChain.ReleaseAndGetAddressOf() );
			if( FAILED( res ) || !m_SwapChain )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to initialize, swap chain couldnt be created" );
				throw std::exception();
			}
			
			// If were debugging, then query for the debug interface
#ifdef HYPERION_DEBUG_RENDERER
			res = m_Device->QueryInterface< ID3D11Debug >( m_Debug.GetAddressOf() );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to get debug interface!" );
				throw std::exception();
			}
#endif

			// Get back buffer texture
			m_BackBuffer = std::shared_ptr< DirectX11Texture2D >( new DirectX11Texture2D() );
			res = m_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**) m_BackBuffer->GetAddress() );

			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to initialize resources.. couldnt get pointer to the back buffer" );
				throw std::exception();
			}

			// Release everything we dont need anymore
			delete[] ptrDisplayModes;
			ptrDisplayModes = nullptr;

			ptrOutput->Release();
			ptrOutput = nullptr;

			ptrAdapter->Release();
			ptrAdapter = nullptr;

			ptrFactory->Release();
			ptrFactory = nullptr;

			// Create render target view
			m_RenderTarget = std::shared_ptr< DirectX11RenderTarget >( new DirectX11RenderTarget( m_BackBuffer, m_Device ) );
			if( !m_RenderTarget || !m_RenderTarget->IsValid() )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to initialize resources.. couldnt create back buffer render target!" );
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
				Console::WriteLine( "[ERROR] DX11: Failed to initialize resources.. couldnt create depth stencil state" );
				throw std::exception();
			}

			// Set active stencil state
			m_bDepthEnabled = true;
			m_DeviceContext->OMSetDepthStencilState( m_DepthStencilState.Get(), 1 );

			// Create Depth Stencil View
			m_DepthStencil = std::dynamic_pointer_cast<DirectX11DepthStencil>( CreateDepthStencil( selectedMode.Width, selectedMode.Height ) );
			if( !m_DepthStencil || !m_DepthStencil->IsValid() )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to initialize resources.. couldnt create depth stencil" );
				throw std::exception();
			}

			// Bind render target view and depth stencil view to the output render view
			ID3D11RenderTargetView *aRenderViews[ 1 ] = { m_RenderTarget->Get() }; // array of pointers
			m_DeviceContext->OMSetRenderTargets( 1, aRenderViews, m_DepthStencil->GetStencilView() );

			// Create the rasterizer
			D3D11_RASTERIZER_DESC rasterDesc;
			ZeroMemory( &rasterDesc, sizeof( rasterDesc ) );

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

			res = m_Device->CreateRasterizerState( &rasterDesc, m_RasterizerState.GetAddressOf() );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to initialize resources.. couldnt create rasterizer" );
				throw std::exception();
			}

			// Set raster state 
			m_DeviceContext->RSSetState( m_RasterizerState.Get() );

			// Setup viewport
			D3D11_VIEWPORT viewport{};
			viewport.Height		= (FLOAT)selectedMode.Height;
			viewport.Width		= (FLOAT)selectedMode.Width;
			viewport.MinDepth	= 0.f;
			viewport.MaxDepth	= 1.f;
			viewport.TopLeftX	= 0.f;
			viewport.TopLeftY	= 0.f;

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
				Console::WriteLine( "[ERROR] DX11: Failed to initialize resources.. couldnt create depth disabled state" );
				throw std::exception();
			}

			// Create blend states
			D3D11_BLEND_DESC blendDesc;
			ZeroMemory( &blendDesc, sizeof( blendDesc ) );

			blendDesc.RenderTarget[ 0 ].BlendEnable				= TRUE;
			blendDesc.RenderTarget[ 0 ].SrcBlend				= D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[ 0 ].DestBlend				= D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[ 0 ].BlendOp					= D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[ 0 ].SrcBlendAlpha			= D3D11_BLEND_ONE;
			blendDesc.RenderTarget[ 0 ].DestBlendAlpha			= D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[ 0 ].BlendOpAlpha			= D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[ 0 ].RenderTargetWriteMask	= 0x0f;

			res = m_Device->CreateBlendState( &blendDesc, m_BlendState.GetAddressOf() );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to initialize resources.. couldnt create normal blend state" );
				throw std::exception();
			}

			blendDesc.RenderTarget[ 0 ].BlendEnable				= FALSE;
			blendDesc.RenderTarget[ 0 ].SrcBlend				= D3D11_BLEND_ONE;
			blendDesc.RenderTarget[ 0 ].DestBlend				= D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[ 0 ].BlendOp					= D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[ 0 ].SrcBlendAlpha			= D3D11_BLEND_ONE;
			blendDesc.RenderTarget[ 0 ].DestBlendAlpha			= D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[ 0 ].BlendOpAlpha			= D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[ 0 ].RenderTargetWriteMask	= 0x0f;

			res = m_Device->CreateBlendState( &blendDesc, m_BlendDisabledState.GetAddressOf() );
			if( FAILED( res ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to initialize resources.. couldnt create alpha disabled blend state" );
				throw std::exception();
			}

			// Set disabled blend state as default
			DisableAlphaBlending();

			// Create screen geometry
			GenerateScreenGeometry( selectedMode.Width, selectedMode.Height );
			GenerateFloorGeometry();

			/* DEBUG TESTS */
			//PerformTiledResourceTests();

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

		Console::WriteLine( "DX11: Resources initialized!" );
		return true;
	}


	void DirectX11Graphics::PerformTiledResourceTests()
	{
		const uint32 tileWidth				= 128;
		const uint32 tileHeight				= 64;
		const uint32 tileSizePixels			= tileWidth * tileHeight;
		const uint32 pixelSizeBytes			= 8;
		const uint32 tileSizeBytes			= tileSizePixels * pixelSizeBytes;
		const uint32 shadowMapCount			= 1; // DEBUG, set to 1
		const uint32 minShadowMipWidth		= 1;
		const uint32 minShadowMipHeight		= 2;
		const uint32 minShadowMipWidthPx	= minShadowMipWidth * tileWidth;
		const uint32 minShadowMipHeightPx	= minShadowMipHeight * tileHeight;
		const uint32 minTileCount			= minShadowMipWidth * minShadowMipHeight * shadowMapCount;
		const uint32 minTileMemory			= minTileCount * tileSizeBytes;
		const uint32 maxShadowMemory		= 128 * 1024 * 1024; // 128MB
		const uint32 maxShadowTiles			= maxShadowMemory / tileSizeBytes;

		// Now lets create the shadow map textures
		D3D11_TEXTURE2D_DESC texDesc {};

		texDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.Usage				= D3D11_USAGE_DEFAULT;
		texDesc.BindFlags			= D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.MiscFlags			= D3D11_RESOURCE_MISC_TILED;
		texDesc.CPUAccessFlags		= 0;
		//texDesc.Width				= 16384;
		//texDesc.Height				= 16384;
		texDesc.Width = 1024;
		texDesc.Height = 1024;
		//texDesc.MipLevels			= 8;
		texDesc.MipLevels = 2;
		texDesc.ArraySize			= 1;
		texDesc.SampleDesc.Count	= 1;
		texDesc.SampleDesc.Quality	= 0;

		for( uint32 i = 0; i < shadowMapCount; i++ )
		{
			auto& mapPtr = m_ShadowMapTextures.emplace_back( nullptr );

			if( FAILED( m_Device->CreateTexture2D( &texDesc, NULL, mapPtr.GetAddressOf() ) ) )
			{
				Console::WriteLine( "[DX11] Shadow Map Test Failed! Couldnt create textures" );
				return;
			}
		}

		// Create the memory pool for the shadow system
		D3D11_BUFFER_DESC bufferDesc {};

		bufferDesc.Usage				= D3D11_USAGE_DEFAULT;
		bufferDesc.MiscFlags			= D3D11_RESOURCE_MISC_TILE_POOL;
		bufferDesc.CPUAccessFlags		= 0;
		bufferDesc.BindFlags			= 0;
		bufferDesc.ByteWidth			= ( maxShadowTiles + 1 ) * tileSizeBytes;
		bufferDesc.StructureByteStride	= bufferDesc.ByteWidth;

		if( FAILED( m_Device->CreateBuffer( &bufferDesc, NULL, m_ShadowMapMemory.GetAddressOf() ) ) )
		{
			Console::WriteLine( "[DX11] Shadow Map Test Failed! Couldnt create the tile pool" );
			return;
		}

		uint32 dummyTileOffsetInTiles = maxShadowTiles;
		uint32 dummyTileOffsetInBytes = maxShadowTiles * tileSizeBytes;

		// Lets get the tiling info for one of these textures, just as a sanity check because the warnings were getting dont make sense!
		
		ComPtr< ID3D11Device2 > device;
		if( FAILED( m_Device->QueryInterface< ID3D11Device2 >( device.GetAddressOf() ) ) )
		{
			Console::WriteLine( "[DX11] Shadow Map Test Failed! Couldnt query higher level device" );
			return;
		}
		/*
		{
			UINT totalTileCount = 0;
			D3D11_PACKED_MIP_DESC packedMipInfo {};
			D3D11_TILE_SHAPE tileShape {};
			UINT numSubresourceTiling = 1;
			D3D11_SUBRESOURCE_TILING subresourceTiling {};

			//UINT subresourceIndex = D3D11CalcSubresource( 5, 0, 8 );
			//Console::WriteLine( "==============> Subresource index should be: ", subresourceIndex );

			device->GetResourceTiling( m_ShadowMapTextures[ 0 ].Get(), &totalTileCount, &packedMipInfo, &tileShape, &numSubresourceTiling, 0, &subresourceTiling );
			Console::WriteLine( "-------" );
		}
		*/

		// Query for a higher version of the device context, so we can call tiled resource api calls
		// TODO: This interface should be used for m_DeviceContext anyway!
		ComPtr< ID3D11DeviceContext2 > deviceContext;
		if( FAILED( m_DeviceContext->QueryInterface< ID3D11DeviceContext2 >( deviceContext.GetAddressOf() ) ) || deviceContext == nullptr )
		{
			Console::WriteLine( "[DX11] Shadow Map Test Fialed! Couldnt query for higher level device context" );
			return;
		}

		// Lets get the tile mappings for the texture, as a test
		

		// Now lets map the first chunk of memory from the buffer, to the lowest mip of each shadow map
		D3D11_TILED_RESOURCE_COORDINATE resourceCoords[ 3 ] {};

		resourceCoords[ 0 ].Subresource = 1;
		resourceCoords[ 0 ].X				= 0;
		resourceCoords[ 0 ].Y				= 0;
		resourceCoords[ 0 ].Z				= 0;

		resourceCoords[ 1 ].Subresource = 1;
		resourceCoords[ 1 ].X = 2;
		resourceCoords[ 1 ].Y = 0;
		resourceCoords[ 1 ].Z = 0;

		resourceCoords[ 2 ].Subresource = 0;
		resourceCoords[ 2 ].X = 0;
		resourceCoords[ 2 ].Y = 0;
		resourceCoords[ 2 ].Z = 0;

		D3D11_TILE_REGION_SIZE regionSize[ 3 ] {};

		regionSize[ 0 ].Width		= 2;	// These are measured in tiles, not pixels
		regionSize[ 0 ].Height		= 4;
		regionSize[ 0 ].Depth		= 1;
		regionSize[ 0 ].NumTiles	= 8;
		regionSize[ 0 ].bUseBox		= true;

		regionSize[ 1 ].Width = 2;	// These are measured in tiles, not pixels
		regionSize[ 1 ].Height = 4;
		regionSize[ 1 ].Depth = 1;
		regionSize[ 1 ].NumTiles = 8;
		regionSize[ 1 ].bUseBox = true;

		regionSize[ 2 ].Width = 8;
		regionSize[ 2 ].Height = 8;
		regionSize[ 2 ].Depth = 1;
		regionSize[ 2 ].NumTiles = 64;
		regionSize[ 2 ].bUseBox = true;

		UINT numResourceRegions		= 3;		 // The number of ranges in the resource were mapping to
		UINT numTileRanges			= 2;		// The number of regions were using in the tile buffer
		UINT rangeFlags[]			= { 0, D3D11_TILE_RANGE_REUSE_SINGLE_TILE };	// Flags for each range of tiles were specifying from the tile buffer
		UINT mappingFlags			= 0;		// D3D11_TILE_MAPPING_FLAGS used for entire operation
		UINT rangeSizes[]			= { 8, 72 }; // Size of each range of tiles were specifying from the tile buffer
		
		auto startTime = std::chrono::high_resolution_clock::now();

		// Were going to map mip level 5, with the top half of the texture being NULL, and the bottom half being mapped to normal memory
		for( uint32 i = 0; i < shadowMapCount; i++ )
		{
			UINT rangeOffsets[] = { 0, dummyTileOffsetInTiles };

			if( FAILED( deviceContext->UpdateTileMappings(
				m_ShadowMapTextures[ i ].Get(),
				numResourceRegions,
				resourceCoords,
				regionSize,
				m_ShadowMapMemory.Get(),
				numTileRanges,
				rangeFlags,
				rangeOffsets,
				rangeSizes,
				mappingFlags
			) ) )
			{
				Console::WriteLine( "[DX11] Shadow Map Test Fialed! Couldnt map dummy tiles to shadow maps!" );
				return;
			}
		}

		auto endTime = std::chrono::high_resolution_clock::now();

		std::chrono::duration< double, std::micro > tileTime = endTime - startTime;
		Console::WriteLine( "================> Tile Mapping Took: ", tileTime.count(), "us" );

		// Finally, lets create the shaders we will use to render shadow map test
		auto shaderFile = FileSystem::OpenFile( FilePath( "shaders/dx11/shadow.hvs", PathRoot::Game ), FileMode::Read );
		auto shaderSize = shaderFile ? shaderFile->GetSize() : 0;

		if( shaderSize <= 0 )
		{
			Console::WriteLine( "[DX11] Shadow Map Test Failed, vertex shader file wasnt found or was invalid" );
			return;
		}

		std::vector< byte > shaderData {};
		{
			DataReader reader( shaderFile );
			reader.SeekBegin();

			if( reader.ReadBytes( shaderData, shaderSize ) != DataReader::ReadResult::Success )
			{
				Console::WriteLine( "[DX11] Shadow Map Test Failed, vertex shader file couldnt be read" );	
				return;
			}
		}

		// Create the shader through DX11
		if( FAILED( m_Device->CreateVertexShader( shaderData.data(), shaderData.size(), NULL, m_ShadowVertexShader.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[DX11] Shadow Map Test Failed, vertex shader couldnt be created" );
			return;
		}

		// Create input layout
		D3D11_INPUT_ELEMENT_DESC inputDesc[ 3 ] {};

		inputDesc[ 0 ].SemanticName = "POSITION";
		inputDesc[ 0 ].SemanticIndex = 0;
		inputDesc[ 0 ].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputDesc[ 0 ].InputSlot = 0;
		inputDesc[ 0 ].AlignedByteOffset = 0;
		inputDesc[ 0 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputDesc[ 0 ].InstanceDataStepRate = 0;

		inputDesc[ 1 ].SemanticName = "NORMAL";
		inputDesc[ 1 ].SemanticIndex = 0;
		inputDesc[ 1 ].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputDesc[ 1 ].InputSlot = 0;
		inputDesc[ 1 ].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		inputDesc[ 1 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputDesc[ 1 ].InstanceDataStepRate = 0;

		inputDesc[ 2 ].SemanticName = "TEXCOORD";
		inputDesc[ 2 ].SemanticIndex = 0;
		inputDesc[ 2 ].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputDesc[ 2 ].InputSlot = 0;
		inputDesc[ 2 ].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		inputDesc[ 2 ].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputDesc[ 2 ].InstanceDataStepRate = 0;

		if( FAILED( m_Device->CreateInputLayout( inputDesc, 3, shaderData.data(), shaderData.size(), m_ShadowInputLayout.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[DX11] Shadow Map Test Failed, couldnt create input layout" );
			return;
		}

		shaderFile = FileSystem::OpenFile( FilePath( "shaders/dx11/shadow.hps", PathRoot::Game ), FileMode::Read );
		shaderSize = shaderFile ? shaderFile->GetSize() : 0;

		if( shaderSize <= 0 )
		{
			Console::WriteLine( "[DX11] Shadow Map Test Failed, pixel shader file wasnt found or was invalid" );
			return;
		}

		shaderData.clear();
		{
			DataReader reader( shaderFile );
			reader.SeekBegin();

			if( reader.ReadBytes( shaderData, shaderSize ) != DataReader::ReadResult::Success )
			{
				Console::WriteLine( "[DX11] Shadow Map Test Failed, pixel shader file couldnt be read" );
				return;
			}
		}

		// Create the shader through DX11
		if( FAILED( m_Device->CreatePixelShader( shaderData.data(), shaderData.size(), NULL, m_ShadowPixelShader.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[DX11] Shadow Map Test Failed, pixel shader couldnt be created" );
			return;
		}

		// We also need a render target for each of our shadow maps
		/*
		for( uint32 i = 0; i < shadowMapCount; i++ )
		{
			auto& rtvList = m_ShadowRenderTargets.emplace_back();

			D3D11_RENDER_TARGET_VIEW_DESC desc {};

			desc.Format			= DXGI_FORMAT_R32G32_FLOAT;
			desc.ViewDimension	= D3D11_RTV_DIMENSION_TEXTURE2D;
			
			for( uint32 j = 0; j < 8; j++ )
			{
				auto& rtv = rtvList.emplace_back();

				desc.Texture2D.MipSlice = j;

				if( FAILED( m_Device->CreateRenderTargetView( m_ShadowMapTextures[ i ].Get(), &desc, rtv.GetAddressOf() ) ) )
				{
					Console::WriteLine( "[DX11] Shadow Map Test Failed! Couldnt create render target view!" );
					return;
				}
			}
		}*/

		D3D11_RENDER_TARGET_VIEW_DESC viewDesc {};

		viewDesc.Format					= DXGI_FORMAT_R8G8B8A8_UNORM;
		viewDesc.ViewDimension			= D3D11_RTV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice		= 1;

		if( FAILED( m_Device->CreateRenderTargetView( m_ShadowMapTextures[ 0 ].Get(), &viewDesc, m_ShadowRenderTarget.GetAddressOf() ) ) )
		{
			Console::WriteLine( "[DX11] Shadow Map Test Failed! Couldnt create render target view" );
		}

		D3D11_BUFFER_DESC shaderBufferDesc {};

		shaderBufferDesc.Usage					= D3D11_USAGE_DYNAMIC;
		shaderBufferDesc.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
		shaderBufferDesc.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;
		shaderBufferDesc.MiscFlags				= 0;
		shaderBufferDesc.ByteWidth				= sizeof( ShadowShaderStaticMatrixType );
		shaderBufferDesc.StructureByteStride	= shaderBufferDesc.ByteWidth;

		if( FAILED( m_Device->CreateBuffer( &shaderBufferDesc, NULL, m_ShadowShaderStaticMatrix.GetAddressOf() ) ) )
		{
			Console::WriteLine( "[DX11] Shadow Map Test Failed! Couldnt create static matrix buffer" );
			return;
		}

		shaderBufferDesc.ByteWidth				= sizeof( ShadowShaderObjectMatrixType );
		shaderBufferDesc.StructureByteStride	= shaderBufferDesc.ByteWidth;

		if( FAILED( m_Device->CreateBuffer( &shaderBufferDesc, NULL, m_ShadowShaderObjectMatrix.GetAddressOf() ) ) )
		{
			Console::WriteLine( "[DX11] Shadow Map Test Failed! Couldnt create object matrix buffer" );
			return;
		}

		Console::WriteLine( "[DX11] Tile mapping test success!" );
	}


	void DirectX11Graphics::PerformShadowTest( BatchCollector& inCollector )
	{
		/*
		// Set shaders
		m_DeviceContext->PSSetShader( m_ShadowPixelShader.Get(), NULL, 0 );
		m_DeviceContext->VSSetShader( m_ShadowVertexShader.Get(), NULL, 0 );
		m_DeviceContext->IASetInputLayout( m_ShadowInputLayout.Get() );

		ID3D11Buffer* bufferList[] = { m_ShadowShaderStaticMatrix.Get(), m_ShadowShaderObjectMatrix.Get() };
		m_DeviceContext->VSSetConstantBuffers( 0, 2, bufferList );

		// Set render target view
		ID3D11RenderTargetView* rtvList[]	= { m_ShadowRenderTarget.Get() };
		FLOAT clearColor[]					= { 0.f, 0.f, 0.f, 0.f };

		m_DeviceContext->ClearRenderTargetView( rtvList[ 0 ], clearColor );
		m_DeviceContext->OMSetRenderTargets( 1, rtvList, NULL );

		// Map the static buffer and input the view and projection matrix
		auto projMatrix = DirectX::XMMatrixTranspose( DirectX::XMMatrixPerspectiveFovLH( DirectX::XM_PIDIV4, 1.f, 0.1f, 1000.f ) );
		auto viewMatrix = DirectX::XMMatrixTranspose( m_ViewMatrix );

		D3D11_MAPPED_SUBRESOURCE resource {};

		if( FAILED( m_DeviceContext->Map( m_ShadowShaderStaticMatrix.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) ) )
		{
			Console::WriteLine( "[DX11] Shadow test error! Failed to map static matrix buffer" );
			return;
		}

		auto* bufferPtr			= (ShadowShaderStaticMatrixType*) resource.pData;
		bufferPtr->View			= viewMatrix;
		bufferPtr->Projection	= projMatrix;

		m_DeviceContext->Unmap( m_ShadowShaderStaticMatrix.Get(), 0 );
		
		// Now lets render the scene
		for( auto it = inCollector.m_OpaqueGroups.begin(); it != inCollector.m_OpaqueGroups.end(); it++ )
		{
			auto& vb = it->second.VertexBuffer;
			auto& ib = it->second.IndexBuffer;

			auto* dx11vb = dynamic_cast< DirectX11Buffer* >( vb.get() );
			auto* dx11ib = dynamic_cast< DirectX11Buffer* >( ib.get() );

			if( dx11vb == nullptr || dx11ib == nullptr ) { continue; }

			m_DeviceContext->IASetIndexBuffer( dx11ib->GetBuffer(), DXGI_FORMAT_R32_UINT, 0 );

			ID3D11Buffer* vbList[] = { dx11vb->GetBuffer() };
			UINT offsets[] = { 0 };
			UINT strides[] = { sizeof( Vertex3D ) };

			m_DeviceContext->IASetVertexBuffers( 0, 1, vbList, strides, offsets );

			for( auto bit = it->second.Batches.begin(); bit != it->second.Batches.end(); bit++ )
			{
				// Map the consatnt buffer for world matrix
				if( FAILED( m_DeviceContext->Map( m_ShadowShaderObjectMatrix.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) ) )
				{
					Console::WriteLine( "[DX11] Shadow test error! Failed to map object matrix buffer" );
					return;
				}

				auto* bufferPtr = (ShadowShaderObjectMatrixType*) resource.pData;

				uint32 count = 0;
				for( uint32 i = 0; i < bit->second.InstanceTransforms.size() && i < 512; i++ )
				{
					bufferPtr->World[ i ] = DirectX::XMMatrixTranspose( DirectX::XMMATRIX( bit->second.InstanceTransforms[ i ].GetData() ) );
					count++;
				}

				m_DeviceContext->Unmap( m_ShadowShaderObjectMatrix.Get(), 0 );

				m_DeviceContext->DrawIndexedInstanced( dx11ib->GetCount(), count, 0, 0, 0 );
			}
		}

		ID3D11RenderTargetView* nullRtvs[] = { NULL };

		m_DeviceContext->OMSetRenderTargets( 1, nullRtvs, NULL );
		m_DeviceContext->VSSetShader( NULL, NULL, 0 );
		m_DeviceContext->PSSetShader( NULL, NULL, 0 );

		D3D11_BOX copyBox {};
		copyBox.top = 0;
		copyBox.left = 0;
		copyBox.right = 512;
		copyBox.bottom = 512;
		copyBox.front = 0;
		copyBox.back = 1;

		m_DeviceContext->CopySubresourceRegion( m_BackBuffer->Get(), 0, 0, 0, 0, m_ShadowMapTextures[ 0 ].Get(), 1, &copyBox );
		*/
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
		for( auto it = m_ShadowMapTextures.begin(); it != m_ShadowMapTextures.end(); it++ )
		{
			it->Reset();
		}
		/*
		for( auto it = m_ShadowRenderTargets.begin(); it != m_ShadowRenderTargets.end(); it++ )
		{
			for( auto j = it->begin(); j != it->end(); j++ )
			{
				j->Reset();
			}
		}
		m_ShadowRenderTargets.clear();
		*/

		m_ShadowRenderTarget.Reset();
		m_ShadowMapTextures.clear();
		m_ShadowMapMemory.Reset();
		m_ShadowVertexShader.Reset();
		m_ShadowPixelShader.Reset();
		m_ShadowShaderObjectMatrix.Reset();
		m_ShadowShaderStaticMatrix.Reset();
		m_ShadowInputLayout.Reset();
		
		if( m_ScreenVertexList )	{ m_ScreenVertexList.Reset(); }
		if( m_ScreenIndexList )		{ m_ScreenIndexList.Reset(); }
		if( m_FloorVertexBuffer )	{ m_FloorVertexBuffer.reset(); }
		if( m_FloorIndexBuffer )	{ m_FloorIndexBuffer.reset(); }
		if( m_BackBuffer )			{ m_BackBuffer.reset(); }
		if( m_RasterizerState )		{ m_RasterizerState.Reset(); }
		if( m_DepthStencilState )	{ m_DepthStencilState.Reset(); }
		if( m_DepthDisabledState )	{ m_DepthDisabledState.Reset(); }
		if( m_DepthStencil )		{ m_DepthStencil.reset(); }
		if( m_BlendState )			{ m_BlendState.Reset(); }
		if( m_BlendDisabledState )	{ m_BlendDisabledState.Reset(); }
		if( m_RenderTarget )		{ m_RenderTarget.reset(); }
		if( m_SwapChain )			{ m_SwapChain->SetFullscreenState( false, nullptr ); m_SwapChain.Reset(); }
		if( m_Device )				{ m_Device.Reset(); }

	#ifdef HYPERION_DEBUG_RENDERER
		if( m_DeviceContext ) { m_DeviceContext->ClearState(); m_DeviceContext->Flush(); }
		if( m_Debug )
		{
			m_Debug->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );
			m_Debug.Reset();
		}
		if( m_DeviceContext ) { m_DeviceContext.Reset(); }
	#else
		if( m_DeviceContext )		{ m_DeviceContext->ClearState(); m_DeviceContext->Flush(); m_DeviceContext.Reset(); }
	#endif
	}


	void DirectX11Graphics::SetCameraInfo( const ViewState& inView )
	{
		// Check if the camera has changed since last update
		if( !m_bRunning || m_CameraPosition.x != inView.Position.X || m_CameraPosition.y != inView.Position.Y || m_CameraPosition.z != inView.Position.Z ||
			m_CameraRotation.m128_f32[ 0 ] != inView.Rotation.X || m_CameraRotation.m128_f32[ 1 ] != inView.Rotation.Y || m_CameraRotation.m128_f32[ 2 ] != inView.Rotation.Z ||
			m_CameraRotation.m128_f32[ 3 ] != inView.Rotation.W || m_FOV != inView.FOV )
		{
			// Update cached camera potiion/rotation
			m_CameraPosition.x = inView.Position.X;
			m_CameraPosition.y = inView.Position.Y;
			m_CameraPosition.z = inView.Position.Z;

			m_CameraRotation = DirectX::XMVectorSet( -inView.Rotation.X, -inView.Rotation.Y, -inView.Rotation.Z, inView.Rotation.W );

			m_FOV = inView.FOV;

			// Re-calculate view matrix
			auto rot = DirectX::XMMatrixRotationQuaternion( m_CameraRotation );

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


	void DirectX11Graphics::TransformAABB( const Transform& inTransform, const AABB& inBounds, OBB& outBounds )
	{
		// Create OBB in place of AABB
		auto bfl = DirectX::XMVectorSet( inBounds.Min.X, inBounds.Min.Y, inBounds.Min.Z, 1.f );
		auto bfr = DirectX::XMVectorSet( inBounds.Max.X, inBounds.Min.Y, inBounds.Min.Z, 1.f );
		auto bbl = DirectX::XMVectorSet( inBounds.Min.X, inBounds.Min.Y, inBounds.Max.Z, 1.f );
		auto bbr = DirectX::XMVectorSet( inBounds.Max.X, inBounds.Min.Y, inBounds.Max.Z, 1.f );
		auto tfl = DirectX::XMVectorSet( inBounds.Min.X, inBounds.Max.Y, inBounds.Min.Z, 1.f );
		auto tfr = DirectX::XMVectorSet( inBounds.Max.X, inBounds.Max.Y, inBounds.Min.Z, 1.f );
		auto tbl = DirectX::XMVectorSet( inBounds.Min.X, inBounds.Max.Y, inBounds.Max.Z, 1.f );
		auto tbr = DirectX::XMVectorSet( inBounds.Max.X, inBounds.Max.Y, inBounds.Max.Z, 1.f );

		// Now we need to rotate these points, and then translate them
		auto worldMatrix = DirectX::XMMatrixRotationQuaternion( DirectX::XMVectorSet( inTransform.Rotation.X, inTransform.Rotation.Y, inTransform.Rotation.Z, inTransform.Rotation.W ) );
		worldMatrix *= DirectX::XMMatrixTranslation( inTransform.Position.X, inTransform.Position.Y, inTransform.Position.Z );

		bfl = DirectX::XMVector3TransformCoord( bfl, worldMatrix );
		bfr = DirectX::XMVector3TransformCoord( bfr, worldMatrix );
		bbl = DirectX::XMVector3TransformCoord( bbl, worldMatrix );
		bbr = DirectX::XMVector3TransformCoord( bbr, worldMatrix );
		tfl = DirectX::XMVector3TransformCoord( tfl, worldMatrix );
		tfr = DirectX::XMVector3TransformCoord( tfr, worldMatrix );
		tbl = DirectX::XMVector3TransformCoord( tbl, worldMatrix );
		tbr = DirectX::XMVector3TransformCoord( tbr, worldMatrix );

		outBounds.BottomFrontLeft	= Vector3D( bfl.m128_f32 );
		outBounds.BottomFrontRight	= Vector3D( bfr.m128_f32 );
		outBounds.BottomBackLeft	= Vector3D( bbl.m128_f32 );
		outBounds.BottomBackRight	= Vector3D( bbr.m128_f32 );
		outBounds.TopFrontLeft		= Vector3D( tfl.m128_f32 );
		outBounds.TopFrontRight		= Vector3D( tfr.m128_f32 );
		outBounds.TopBackLeft		= Vector3D( tbl.m128_f32 );
		outBounds.TopBackRight		= Vector3D( tbr.m128_f32 );
	}


	bool DirectX11Graphics::CheckViewCull( ProxyPrimitive& inPrimitive )
	{
		// We perform a test, and if the test fails against any of the planes, we return the plane it failed against
		// We store that value, and next iteration we check that plane first for optimization
		// So, if the result is negative, it means the test passed all planes, and the cull test passed
		int result = m_ViewFrustum.PerformCoherencyOBBTest( inPrimitive.m_OrientedBounds, inPrimitive.m_CoherencyState );
		inPrimitive.m_CoherencyState = result;
		
		return result < 0;
	}

	bool DirectX11Graphics::CheckViewCull( ProxyLight& inLight )
	{
		// TODO: Check if its more efficient to just have light culling occur on the GPU, or if the performance hit on the CPU
		// side is offset by GPU gains, which is very questionable
		// We have two ways to cull lights against a view frustum...
		// 1. Construct an AABB from the origin and radius [(pos.x - radius, pos.y - radius, pos.z - radius), (pos.x + radius, pos.y + radius, pos.z + radius)]
		// 2. Perform an actual Sphere-Frustum test
		// To optimize, we can store some state in the light, to indicate which plane to recheck next pass

		// Depending on how the shadowing algorithm ends up being built, performing culling on the lights might be more effective...

		return true;
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
		auto pos			= DirectX::XMVectorSet( 0.f, 0.f, -0.1f, 1.f ); // TODO: In another project, I had this as -0.1f

		// Generate static matricies
		m_ProjectionMatrix		= DirectX::XMMatrixPerspectiveFovLH( inFOV, aspectRatio, inNear, inFar );
		m_WorldMatrix			= DirectX::XMMatrixIdentity();
		m_OrthoMatrix			= DirectX::XMMatrixOrthographicLH( (float)inRes.Width, (float)inRes.Height, inNear, inFar );
		m_ScreenViewMatrix		= DirectX::XMMatrixLookAtLH( pos, lookAt, up );
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


	void DirectX11Graphics::GenerateFloorGeometry()
	{
		Vertex3D verts[ 4 ]{};

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

		auto vertBuffer		= std::shared_ptr< DirectX11Buffer >( new DirectX11Buffer( BufferType::Vertex ) );
		auto indexBuffer	= std::shared_ptr< DirectX11Buffer >( new DirectX11Buffer( BufferType::Index ) );

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

		vertBuffer->m_Count		= 4;
		vertBuffer->m_Size		= sizeof( Vertex3D ) * 4;

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

		indexBuffer->m_Count	= 6;
		indexBuffer->m_Size		= sizeof( uint32 ) * 6;

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
			Console::WriteLine( "[ERROR] DX11: Failed to enable alpha blending" );
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
			Console::WriteLine( "[ERROR] DX11: Failed to disable alpha blending" );
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
			Console::WriteLine( "[ERROR] DX11: Failed to check if alpha blending is enabled" );
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
			Console::WriteLine( "[ERROR] DX11: Failed to enable Z buffer" );
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
			Console::WriteLine( "[ERROR] DX11: Failed to dsiable Z buffer" );
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
			Console::WriteLine( "[ERROR] DX11: Failed to check if Z buffer is enabled" );
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

		newBuffer->m_Size				= inParams.Size;
		newBuffer->m_Count				= inParams.Count;
		newBuffer->m_AssetIdentifier	= inParams.SourceAsset;

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
		if( HYPERION_HAS_FLAG( inParams.BindTargets, RENDERER_TEXTURE_BIND_FLAG_SHADER ) )
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

		Output->m_AssetIdentifier = inParams.AssetIdentifier;
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
		if( HYPERION_HAS_FLAG( inParams.BindTargets, RENDERER_TEXTURE_BIND_FLAG_SHADER ) )
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

		Output->m_AssetIdentifier = inParams.AssetIdentifier;
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
		if( HYPERION_HAS_FLAG( inParams.BindTargets, RENDERER_TEXTURE_BIND_FLAG_SHADER ) )
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

		Output->m_AssetIdentifier = inParams.AssetIdentifier;
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
		ClearRenderTarget( inTarget, inColor.r, inColor.g, inColor.b, inColor.a );
	}


	void DirectX11Graphics::ClearRenderTarget( const std::shared_ptr< RRenderTarget >& inTarget, float inR, float inG, float inB, float inA )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Context was null" );

		if( !inTarget || !inTarget->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to clear render target, it was null/invalid" );
			return;
		}

		auto* targetPtr = dynamic_cast<DirectX11RenderTarget*>( inTarget.get() );
		HYPERION_VERIFY( targetPtr, "[DX11] Failed to cast render target to correct type" );

		FLOAT color[ 4 ] = { inR, inG, inB, inA };

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


	std::shared_ptr< RViewClusters > DirectX11Graphics::CreateViewClusters()
	{
		auto output = std::make_shared< DirectX11ViewClusters >();
		if( !output->Initialize( m_Device.Get() ) )
		{
			return nullptr;
		}

		return output;
	}


	std::shared_ptr< RLightBuffer > DirectX11Graphics::CreateLightBuffer()
	{
		auto output = std::make_shared< DirectX11LightBuffer >( RENDERER_MAX_DYNAMIC_LIGHTS );
		if( !output->Initialize( m_Device.Get(), m_DeviceContext.Get() ) )
		{
			return nullptr;
		}

		return output;
	}


	/*
	*	Rendering
	*/
	void DirectX11Graphics::SetNoRenderTargetAndClusterWriteAccess( const std::shared_ptr< RViewClusters >& inClusters )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Device context was null" );

		auto* dx11Clusters = dynamic_cast<DirectX11ViewClusters*>( inClusters.get() );
		auto* clusterUAV = dx11Clusters ? dx11Clusters->GetClusterInfoUAV() : nullptr;

		if( clusterUAV == nullptr )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to set render target and cluster write access, the cluster info UAV was null!" );
			return;
		}

		ID3D11UnorderedAccessView* uavList[]		= { clusterUAV };
		ID3D11RenderTargetView* renderTargets[]		= { NULL };

		m_DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews( 0, renderTargets, NULL, 0, 1, uavList, NULL );
	}


	void DirectX11Graphics::ClearClusterWriteAccess()
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Device context was null" );

		ID3D11UnorderedAccessView* uavList[] = { NULL };
		ID3D11RenderTargetView* targetList[] = { NULL };

		m_DeviceContext->OMSetRenderTargetsAndUnorderedAccessViews( 0, targetList, NULL, 0, 1, uavList, NULL );
	}


	void DirectX11Graphics::SetGBufferRenderTarget( const std::shared_ptr< GBuffer >& inGBuffer, const std::shared_ptr< RDepthStencil >& inStencil )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Device context was null" );

		if( !inGBuffer || !inGBuffer->IsValid() || !inStencil || !inStencil->IsValid() )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to set render output to GBuffer, it was null/invalid" );
			return;
		}

		// Cast each texture to the DX11 version...
		auto* diffPtr		= dynamic_cast< DirectX11RenderTarget* >( inGBuffer->GetDiffuseRoughnessTarget().get() );
		auto* normPtr		= dynamic_cast< DirectX11RenderTarget* >( inGBuffer->GetNormalDepthTarget().get() );
		auto* specPtr		= dynamic_cast< DirectX11RenderTarget* >( inGBuffer->GetSpecularTarget().get() );
		auto* stencilPtr	= dynamic_cast< DirectX11DepthStencil* >( inStencil.get() );

		HYPERION_VERIFY( diffPtr && normPtr && specPtr && stencilPtr, "[DX11] G-Buffer render targets were null" );

		// Set render targets with output manager
		ID3D11RenderTargetView* targetList[] = { diffPtr->Get(), normPtr->Get(), specPtr->Get() };
		m_DeviceContext->OMSetRenderTargets( 3, targetList, stencilPtr->GetStencilView() );

		// Set viewport
		D3D11_VIEWPORT viewport {};

		viewport.Width		= (float) m_Resolution.Width;
		viewport.Height		= (float) m_Resolution.Height;
		viewport.MinDepth	= 0.f;
		viewport.MaxDepth	= 1.f;
		viewport.TopLeftX	= 0.f;
		viewport.TopLeftY	= 0.f;

		HYPERION_VERIFY( viewport.Width > 0 && viewport.Height > 0, "[DX11] Invalid resolution" );

		m_DeviceContext->RSSetViewports( 1, &viewport );
	}


	void DirectX11Graphics::DetachRenderTarget()
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Device context was null" );

		ID3D11RenderTargetView* targetList[] = { NULL, NULL, NULL };
		m_DeviceContext->OMSetRenderTargets( 3, targetList, NULL );
	}

	void DirectX11Graphics::SetRenderTarget( const std::shared_ptr<RRenderTarget>& inTarget, const std::shared_ptr<RDepthStencil>& inStencil )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Device context was null" );

		// We need to cast to the proper types
		if( !inTarget || !inTarget->IsValid() )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to set render target, it was null" );
			return;
		}

		auto* dx11Target	= dynamic_cast< DirectX11RenderTarget* >( inTarget.get() );
		auto* dx11Stencil	= dynamic_cast< DirectX11DepthStencil* >( inStencil.get() );

		ID3D11RenderTargetView* targetList[] = { dx11Target->Get() };
		m_DeviceContext->OMSetRenderTargets( 1, targetList, dx11Stencil ? dx11Stencil->GetStencilView() : NULL );

		// Set viewport
		D3D11_VIEWPORT viewport {};

		viewport.Width		= (float) m_Resolution.Width;
		viewport.Height		= (float) m_Resolution.Height;
		viewport.MinDepth	= 0.f;
		viewport.MaxDepth	= 1.f;
		viewport.TopLeftX	= 0.f;
		viewport.TopLeftY	= 0.f;

		HYPERION_VERIFY( viewport.Width > 0 && viewport.Height > 0, "[DX11] Invalid resolution" );

		m_DeviceContext->RSSetViewports( 1, &viewport );
	}


	void DirectX11Graphics::RenderGeometry( const std::shared_ptr<RBuffer>& inIndexBuffer, const std::shared_ptr<RBuffer>& inVertexBuffer, uint32 inIndexCount )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Device context was null" );
		
		// Ensure we have everything we need to actually render
		if( inIndexBuffer == nullptr || inVertexBuffer == nullptr || inIndexCount < 3 ) { return; }

		UploadGeometry( inIndexBuffer, inVertexBuffer );
		m_DeviceContext->DrawIndexed( inIndexCount, 0, 0 );
	}


	void DirectX11Graphics::UploadGeometry( const std::shared_ptr<RBuffer>& inIndexBuffer, const std::shared_ptr<RBuffer>& inVertexBuffer )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Device context was null" );

		// Ensure we have the resources we need
		if( inIndexBuffer == nullptr || inVertexBuffer == nullptr ) { return; }

		// If this geometry is already attached to the render pipeline, then dont bother re-attaching it
		auto geometryAssetIdentifier = inIndexBuffer->GetAssetIdentifier();
		if( geometryAssetIdentifier == ASSET_INVALID || geometryAssetIdentifier != m_AttachedGeometryAssetIdentifier )
		{
			auto* vertexBuffer = dynamic_cast<DirectX11Buffer*>( inVertexBuffer.get() );
			auto* dx11Vertex = vertexBuffer ? vertexBuffer->GetBuffer() : nullptr;

			if( dx11Vertex == nullptr ) { return; }

			auto* indexBuffer = dynamic_cast<DirectX11Buffer*>( inIndexBuffer.get() );
			auto* dx11Index = indexBuffer ? indexBuffer->GetBuffer() : nullptr;

			if( dx11Index == nullptr ) { return; }

			ID3D11Buffer* vertexBufferList[] = { dx11Vertex };
			UINT strides[] = { sizeof( Vertex3D ) };
			UINT offsets[] = { 0 };

			m_DeviceContext->IASetVertexBuffers( 0, 1, vertexBufferList, strides, offsets );
			m_DeviceContext->IASetIndexBuffer( dx11Index, DXGI_FORMAT_R32_UINT, 0 );

			m_AttachedGeometryAssetIdentifier = geometryAssetIdentifier;
		}
	}


	void DirectX11Graphics::RenderBatch( uint32 inInstanceCount, uint32 inIndexCount )
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Device context was null" );

		if( inInstanceCount == 0 || inIndexCount < 3 ) { return; }

		m_DeviceContext->DrawIndexedInstanced( inIndexCount, inInstanceCount, 0, 0, 0 );
	}


	void DirectX11Graphics::RenderScreenQuad()
	{
		HYPERION_VERIFY( m_DeviceContext, "[DX11] Device context was null" );

		ID3D11Buffer* bufferList[]	= { m_ScreenVertexList.Get() };
		UINT vertexStrides[]		= { sizeof( WindowVertex ) };
		UINT vertexOffsets[]		= { 0 };

		m_DeviceContext->IASetVertexBuffers( 0, 1, bufferList, vertexStrides, vertexOffsets );
		m_DeviceContext->IASetIndexBuffer( m_ScreenIndexList.Get(), DXGI_FORMAT_R32_UINT, 0 );

		m_AttachedGeometryAssetIdentifier = ASSET_INVALID;
		
		m_DeviceContext->DrawIndexed( 6, 0, 0 );
	}


	void DirectX11Graphics::GetDebugFloorQuad( std::shared_ptr< RBuffer >& outVertexBuffer, std::shared_ptr< RBuffer >& outIndexBuffer, std::vector< Matrix >& outMatricies )
	{
		// Just outputs the buffers..
		// TODO: This is dirty
		outVertexBuffer		= m_FloorVertexBuffer;
		outIndexBuffer		= m_FloorIndexBuffer;
		outMatricies		= m_FloorMatricies;
	}


	void DirectX11Graphics::CalculateViewMatrix( const ViewState& inView, Matrix& outViewMatrix )
	{
		// Create rotation quaternion
		auto rotation = DirectX::XMMatrixRotationQuaternion( DirectX::XMVectorSet( -inView.Rotation.X, -inView.Rotation.Y, -inView.Rotation.Z, inView.Rotation.W ) );

		// Create needed vectors
		auto upVec		= DirectX::XMVectorSet( 0.f, 1.f, 0.f, 1.f );
		auto dirVec		= DirectX::XMVectorSet( 0.f, 0.f, 1.f, 1.f );
		auto camPos		= DirectX::XMVectorSet( inView.Position.X, inView.Position.Y, inView.Position.Z, 0.f );

		// Transform our vectors based on camera transform
		dirVec	= DirectX::XMVector3TransformCoord( dirVec, rotation );
		dirVec	= DirectX::XMVectorAdd( dirVec, camPos );
		upVec	= DirectX::XMVector3TransformCoord( upVec, rotation );

		// Build and return matrix
		auto output = DirectX::XMMatrixLookAtLH( camPos, dirVec, upVec );
		outViewMatrix.AssignData( output.r[ 0 ].m128_f32 );
	}


	void DirectX11Graphics::CalculateProjectionMatrix( const ScreenResolution& inResolution, float inFOV, Matrix& outProjMatrix )
	{
		float aspectRatio	= (float) inResolution.Width / (float) inResolution.Height;
		auto output			= DirectX::XMMatrixPerspectiveFovLH( inFOV, aspectRatio, SCREEN_NEAR, SCREEN_FAR );

		outProjMatrix.AssignData( output.r[ 0 ].m128_f32 );
	}


	void DirectX11Graphics::CalculateWorldMatrix( const Transform& inTransform, Matrix& outWorldMatrix )
	{
		auto output		= DirectX::XMMatrixIdentity();
		output			*= DirectX::XMMatrixRotationQuaternion( DirectX::XMVectorSet( -inTransform.Rotation.X, -inTransform.Rotation.Y, -inTransform.Rotation.Z, -inTransform.Rotation.W ) );
		output			*= DirectX::XMMatrixTranslation( inTransform.Position.X, inTransform.Position.Y, inTransform.Position.Z );

		outWorldMatrix.AssignData( output.r[ 0 ].m128_f32 );
	}


	void DirectX11Graphics::CalculateOrthoMatrix( const ScreenResolution& inResolution, Matrix& outOrthoMatrix )
	{
		auto output = DirectX::XMMatrixOrthographicLH( (float) inResolution.Width, (float) inResolution.Height, SCREEN_NEAR, SCREEN_FAR );
		outOrthoMatrix.AssignData( output.r[ 0 ].m128_f32 );
	}


	void DirectX11Graphics::CalculateScreenViewMatrix( Matrix& outMatrix )
	{
		auto upVec = DirectX::XMVectorSet( 0.f, 1.f, 0.f, 1.f );
		auto dirVec = DirectX::XMVectorSet( 0.f, 0.f, 1.f, 1.f );
		auto posVec = DirectX::XMVectorSet( 0.f, 0.f, 0.f, 1.f );

		auto output = DirectX::XMMatrixLookAtLH( posVec, dirVec, upVec );
		outMatrix.AssignData( output.r[ 0 ].m128_f32 );
	}


	std::shared_ptr< RVertexShader > DirectX11Graphics::CreateVertexShader( VertexShaderType inType )
	{
		switch( inType )
		{
		case VertexShaderType::Scene:
		{
			auto newShader = std::make_shared< DX11SceneVertexShader >();
			return newShader->Initialize( m_Device.Get(), m_DeviceContext.Get() ) ? newShader : nullptr;
			break;
		}
		case VertexShaderType::Screen:
		{
			auto newShader = std::make_shared< DX11ScreenVertexShader >();
			return newShader->Initialize( m_Device.Get(), m_DeviceContext.Get() ) ? newShader : nullptr;
			break;
		}
		default:
			return nullptr;
		}
	}


	std::shared_ptr< RGeometryShader > DirectX11Graphics::CreateGeometryShader( GeometryShaderType inType )
	{
		return nullptr;
	}


	std::shared_ptr< RPixelShader > DirectX11Graphics::CreatePixelShader( PixelShaderType inType )
	{
		switch( inType )
		{
		case PixelShaderType::GBuffer:
		{
			auto newShader = std::make_shared< DX11GBufferPixelShader >();
			return newShader->Initialize( m_Device.Get(), m_DeviceContext.Get() ) ? newShader : nullptr;
			break;
		}
		case PixelShaderType::Lighting:
		{
			auto newShader = std::make_shared< DX11LightingPixelShader >();
			return newShader->Initialize( m_Device.Get(), m_DeviceContext.Get() ) ? newShader : nullptr;
			break;
		}
		case PixelShaderType::ForwardPreZ:
		{
			auto newShader = std::make_shared< DX11ForwardPreZShader >();
			return newShader->Initialize( m_Device.Get(), m_DeviceContext.Get() ) ? newShader : nullptr;
			break;
		}
		case PixelShaderType::Forward:
		{
			auto newShader = std::make_shared< DX11ForwardPixelShader >();
			return newShader->Initialize( m_Device.Get(), m_DeviceContext.Get() ) ? newShader : nullptr;
			break;
		}
		default:
			return nullptr;
		}
	}


	std::shared_ptr< RComputeShader > DirectX11Graphics::CreateComputeShader( ComputeShaderType inType )
	{
		switch( inType )
		{
		case ComputeShaderType::BuildClusters:
		{
			auto newShader = std::make_shared< DX11BuildClustersShader >();
			return newShader->Initialize( m_Device.Get(), m_DeviceContext.Get() ) ? newShader : nullptr;
			break;
		}
		case ComputeShaderType::FindClusters:
		{
			auto newShader = std::make_shared< DX11FindClustersShader >();
			return newShader->Initialize( m_Device.Get(), m_DeviceContext.Get() ) ? newShader : nullptr;
			break;
		}
		case ComputeShaderType::CullLights:
		{
			auto newShader = std::make_shared< DX11CullLightsShader >();
			return newShader->Initialize( m_Device.Get(), m_DeviceContext.Get() ) ? newShader : nullptr;
			break;
		}
		default:
			return nullptr;
		}
	}

	std::shared_ptr<RPostProcessShader> DirectX11Graphics::CreatePostProcessShader( PostProcessShaderType inType )
	{
		switch( inType )
		{
		case PostProcessShaderType::FXAA:
			auto newShader = std::make_shared< DX11FXAAShader >();
			return newShader->Initialize( m_Device.Get(), m_DeviceContext.Get() ) ? newShader : nullptr;
		}

		return nullptr;
	}

}