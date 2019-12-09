/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/DirectX11Renderer.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Hyperion.h"
#ifdef HYPERION_SUPPORT_DIRECTX

#include "Hyperion/Renderer/DirectX11/DirectX11Renderer.h"
#include <iostream>

// DirectX Includes
#include <CommonStates.h>
//#include <Effects.h>


namespace Hyperion
{

	DirectX11Renderer::DirectX11Renderer()
		: Renderer(), m_VSync( false ), m_RenderTarget( NULL ), m_VideoCardMemory( 0 ), m_VideoCardDescription()
	{
	}

	DirectX11Renderer::~DirectX11Renderer()
	{
		Stop();
	}

	bool DirectX11Renderer::SetScreenResolution( const ScreenResolution& inResolution )
	{
		// First, check if were running
		if( m_isRunning && m_SwapChain )
		{
			// Get output device
			IDXGIOutput* output = nullptr;
			if( FAILED( m_SwapChain->GetContainingOutput( &output ) ) )
			{
				std::cout << "[ERROR] DX11Renderer: Failed to update resolution.. couldnt get output!\n";
				return false;
			}

			// Get list of supported display modes on this output device
			uint32 numberModes = 0;
			if( FAILED( output->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numberModes, NULL ) ) )
			{
				std::cout << "[ERROR] DX11Renderer: Failed to update reoslution.. couldnt get display modes list\n";
				output->Release();
				return false;
			}

			// Create array of supported modes
			DXGI_MODE_DESC* supportedModes = new DXGI_MODE_DESC[ numberModes ];
			ZeroMemory( supportedModes, sizeof( DXGI_MODE_DESC ) * numberModes );

			// Fill out this array
			if( FAILED( output->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numberModes, supportedModes ) ) )
			{
				std::cout << "[ERROR] DX11Renderer: Failed to update resolution.. couldnt fill display modes list!\n";
				output->Release();
				delete[] supportedModes;
				return false;
			}

			// Release output device
			output->Release();

			// Now check if this resolution is supported
			bool bSupported = false;
			DXGI_MODE_DESC supportedMode;

			for( uint32 i = 0; i < numberModes; i++ )
			{
				if( supportedModes[ i ].Width == inResolution.Width &&
					supportedModes[ i ].Height == inResolution.Height )
				{
					bSupported = true;
					supportedMode = supportedModes[ i ];
				}
			}

			// Free the list
			delete[] supportedModes;

			if( !bSupported )
			{
				std::cout << "[ERROR] DX11Renderer: Attempt to change reoslution to " << inResolution.Width << "x" << inResolution.Height << " but this mode isnt supported!\n";
				return false;
			}

			// Now, perform resolution update



		}
		else
		{
			m_Resolution = inResolution;
			return true;
		}


		// TODO: Validate if this resolution is supported
		// If we havent init yet, then were just going to set it and on init check it
		// and if were running, then call functions to update resolution while running!
		m_Resolution = inResolution;
		return true;
	}

	std::vector< ScreenResolution > DirectX11Renderer::GetAvailableResolutions()
	{
		// TODO: Build a list of supported resolutions
		return std::vector< ScreenResolution >();
	}

	bool DirectX11Renderer::SetRenderTarget( HWND inTarget )
	{
		// TODO: Check if were running
		if( !inTarget )
			return false;

		m_RenderTarget = inTarget;
		return true;
	}

	void DirectX11Renderer::SetVSync( bool inVSync )
	{
		// TODO: Check if were running
		m_VSync = inVSync;
	}



	bool DirectX11Renderer::Init()
	{
		std::cout << "[DEBUG] DirectX11Renderer: Initialize...\n";

		// Get the parameters we need to startup
		auto renderTarget = GetRenderTarget();
		if( !renderTarget )
		{
			std::cout << "[ERROR] DX11Renderer: Failed to initialize.. render target is not set!\n";
		}

		// If any resources are still open.. then shut them down
		ShutdownResources();

		// Initialize our new resources
		auto resolution = GetScreenResolution();
		if( !InitializeResources( renderTarget, resolution ) )
		{
			return false;
		}

		// The resolution might have been updated if the one we passed in wasnt supported
		// So, we need to update our stored resolution to this new value
		m_Resolution = resolution;

		// Generate matricies needed for rendering
		GenerateMatricies( resolution, DirectX::XM_PIDIV4, SCREEN_NEAR, SCREEN_FAR );

		// TODO: Stuff


		return true;
	}


	void DirectX11Renderer::GenerateMatricies( const ScreenResolution& inRes, float inFOV, float inNear, float inFar )
	{
		// Calculate values needed to generate matricies
		float Aspect = (float) inRes.Width / (float) inRes.Height;

		// Create matricies
		m_ProjectionMatrix	= DirectX::XMMatrixPerspectiveFovLH( inFOV, Aspect, inNear, inFar );
		m_WorldMatrix		= DirectX::XMMatrixIdentity();
		m_OrthoMatrix		= DirectX::XMMatrixOrthographicLH( (float) inRes.Width, (float) inRes.Height, inNear, inFar );
		
		auto up			= DirectX::XMVectorSet( 0.f, 1.f, 0.f, 1.f );
		auto lookAt		= DirectX::XMVectorSet( 0.f, 0.f, 1.f, 1.f );
		auto position	= DirectX::XMFLOAT3( 0.f, 0.f, -0.1f );

		m_ScreenMatrix = DirectX::XMMatrixLookAtLH( DirectX::XMLoadFloat3( &position ), lookAt, up );

	}


	bool DirectX11Renderer::InitializeResources( HWND renderTarget, ScreenResolution& inRes )
	{
		// Validate parameters
		if( !renderTarget )
		{
			std::cout << "[ERROR] DX11Renderer: failed to initialize resources.. render target was invalid\n";
			return false;
		}

		// TODO: Validate resolution

		// Create pointers to dxgi resources we will create in our try block
		IDXGIFactory* ptrFactory			= nullptr;
		IDXGIAdapter* ptrAdapter			= nullptr;
		IDXGIOutput* ptrOutput				= nullptr;
		DXGI_MODE_DESC* ptrDisplayModes		= nullptr;
		ID3D11Texture2D* ptrBackBuffer		= nullptr;
		
		try
		{
			// Create graphics interface factory
			HRESULT res = CreateDXGIFactory( __uuidof( IDXGIFactory ), (void**) &ptrFactory );
			if( FAILED( res ) || !ptrFactory )
			{
				std::cout << "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create graphics interface factory\n";
				throw std::exception();
			}

			// Create video device adapter
			res = ptrFactory->EnumAdapters( 0, &ptrAdapter );
			if( FAILED( res ) || !ptrAdapter )
			{
				std::cout << "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt enumerate adapters\n";
				throw std::exception();
			}

			res = ptrAdapter->EnumOutputs( 0, &ptrOutput );
			if( FAILED( res ) || !ptrOutput )
			{
				std::cout << "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt enum outputs!\n";
				throw std::exception();
			}

			unsigned int numModes = 0;
			res = ptrOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL );
			if( FAILED( res ) )
			{
				std::cout << "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt get display modes list\n";
				throw std::exception();
			}

			ptrDisplayModes = new DXGI_MODE_DESC[ numModes ];
			res = ptrOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, ptrDisplayModes );
			if( FAILED( res ) )
			{
				std::cout << "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt fill display mode list\n";
				throw std::exception();
			}

			if( numModes <= 0 )
			{
				std::cout << "[ERROR] DX11Renderer: There are no supported display modes! Failed to initialize\n";
				throw std::exception();
			}

			// Now that we have a list of supported resolutions.. lets check if the desired one is supported
			bool bResolutionSupported = false;
			DXGI_MODE_DESC selectedMode;

			for( uint32 i = 0; i < numModes; i++ )
			{
				if( ptrDisplayModes[ i ].Width == inRes.Width &&
					ptrDisplayModes[ i ].Height == inRes.Height )
				{
					bResolutionSupported = true;
					selectedMode = ptrDisplayModes[ i ];
				}
			}

			if( !bResolutionSupported )
			{
				// We need to default to one of the modes in the list
				std::cout << "[WARNING] DX11Renderer: Selected resolution wasnt supported.. defaulting!\n";
				selectedMode = ptrDisplayModes[ 0 ];
			}

			// Update stored resolution to the supported one
			inRes.Width			= selectedMode.Width;
			inRes.Height			= selectedMode.Height;

			// Get video card description
			DXGI_ADAPTER_DESC videoCardDescription;
			ZeroMemory( &videoCardDescription, sizeof( videoCardDescription ) );

			res = ptrAdapter->GetDesc( &videoCardDescription );
			if( FAILED( res ) )
			{
				std::cout << "[ERROR] DX11Renderer: Failed to initialize resources.. coudlnt read video card information\n";
				throw std::exception();
			}

			m_VideoCardMemory		= (uint32)( videoCardDescription.DedicatedVideoMemory / 1024 / 1024 );
			m_VideoCardDescription	= String( std::wstring( videoCardDescription.Description ), StringEncoding::UTF16 );

			// Release everything we dont need anymore
			delete[] ptrDisplayModes;
			ptrDisplayModes = nullptr;

			ptrOutput->Release();
			ptrOutput = nullptr;

			ptrAdapter->Release();
			ptrAdapter = nullptr;

			ptrFactory->Release();
			ptrFactory = nullptr;

			// Create swap chain
			DXGI_SWAP_CHAIN_DESC swapChainDesc;
			ZeroMemory( &swapChainDesc, sizeof( swapChainDesc ) );

			// TODO: Multi outputs?
			swapChainDesc.BufferCount			= 1;
			swapChainDesc.BufferDesc			= selectedMode;
			swapChainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.OutputWindow			= renderTarget;
			swapChainDesc.SampleDesc.Count		= 1;
			swapChainDesc.SampleDesc.Quality	= 0;
			swapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT_DISCARD;
			swapChainDesc.Flags					= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			swapChainDesc.Windowed				= !inRes.FullScreen;

			// If VSync isnt set, then unlock the refresh rate
			if( !GetVSyncEnabled() )
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
				std::cout << "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create swap chain and device!\n";
				throw std::exception();
			}

			// Get pointer to the back buffer
			res = m_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**) &ptrBackBuffer );

			if( FAILED( res ) || !ptrBackBuffer )
			{
				std::cout << "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt get pointer to the back buffe\n";
				throw std::exception();
			}

			// Create render target view
			res = m_Device->CreateRenderTargetView( ptrBackBuffer, NULL, m_RenderTargetView.GetAddressOf() );
			if( FAILED( res ) )
			{
				std::cout << "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create render target view\n";
				throw std::exception();
			}

			// Release back buffer pointer
			ptrBackBuffer->Release();
			ptrBackBuffer = nullptr;

			// Create common states object
			m_CommonStates = std::make_unique< DirectX::CommonStates >( m_Device.Get() );

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
				std::cout << "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create depth buffer\n";
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
				std::cout << "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create depth stencil state\n";
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
				std::cout << "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create depth stencil view\n";
				throw std::exception();
			}

			// Bind render target view and depth stencil view to the output render view
			ID3D11RenderTargetView *aRenderViews[ 1 ] = { m_RenderTargetView.Get() }; // array of pointers
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
				std::cout << "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create rasterizer\n";
				throw std::exception();
			}

			// Set raster state 
			m_DeviceContext->RSSetState( m_RasterizerState.Get() );

			// Setup viewport
			D3D11_VIEWPORT viewport;
			viewport.Height		= selectedMode.Height;
			viewport.Width		= selectedMode.Width;
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
				std::cout << "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create depth disabled state\n";
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
				std::cout << "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create normal blend state\n";
				throw std::exception();
			}

			blendDesc.RenderTarget[ 0 ].BlendEnable = FALSE;

			res = m_Device->CreateBlendState( &blendDesc, m_BlendDisabledState.GetAddressOf() );
			if( FAILED( res ) )
			{
				std::cout << "[ERROR] DX11Renderer: Failed to initialize resources.. couldnt create alpha disabled blend state\n";
				throw std::exception();
			}

			// Set disabled blend state as default
			float blendFactor[ 4 ] = { 0.f, 0.f, 0.f, 0.f };
			UINT sampleMask = 0xFFFFFFFF;

			m_DeviceContext->OMSetBlendState( m_BlendDisabledState.Get(), blendFactor, sampleMask );
		}
		catch( ... )
		{
			if( ptrFactory ) { ptrFactory->Release(); ptrFactory = nullptr; }
			if( ptrAdapter ) { ptrAdapter->Release(); ptrAdapter = nullptr; }
			if( ptrOutput ) { ptrOutput->Release(); ptrOutput = nullptr; }
			if( ptrDisplayModes ) { delete[] ptrDisplayModes; ptrDisplayModes = nullptr; }
			if( ptrBackBuffer ) { ptrBackBuffer->Release(); ptrBackBuffer = nullptr; }

			ShutdownResources();
			return false;
		}

		std::cout << "[GOOD] DX11Renderer: Resource initialization complete!\n";
		return true;
	}


	void DirectX11Renderer::ShutdownResources()
	{
		if( m_CommonStates )		{ m_CommonStates.reset(); }
		//if( m_EffectFactory )		{ m_EffectFactory->ReleaseCache(); m_EffectFactory.reset(); }
		if( m_RasterizerState )		{ m_RasterizerState.Reset(); }
		if( m_DepthStencilView )	{ m_DepthStencilView.Reset(); }
		if( m_DepthStencilState )	{ m_DepthStencilState.Reset(); }
		if( m_DepthDisabledState )	{ m_DepthDisabledState.Reset(); }
		if( m_DepthStencilBuffer )	{ m_DepthStencilBuffer.Reset(); }
		if( m_BlendState )			{ m_BlendState.Reset(); }
		if( m_BlendDisabledState )	{ m_BlendDisabledState.Reset(); }
		if( m_RenderTargetView )	{ m_RenderTargetView.Reset(); }
		if( m_DeviceContext )		{ m_DeviceContext.Reset(); }
		if( m_Device )				{ m_Device.Reset(); }
		if( m_SwapChain )			{ m_SwapChain->SetFullscreenState( false, nullptr ); m_SwapChain.Reset(); }
	}


	void DirectX11Renderer::Frame()
	{

	}

	void DirectX11Renderer::Shutdown()
	{
		std::cout << "[DEBUG] DirectX11Renderer: Shutdown...\n";

		// Shutdown any active resources
		ShutdownResources();
	}



}


#endif