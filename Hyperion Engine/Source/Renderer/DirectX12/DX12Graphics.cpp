/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX12/DX12Graphics.cpp
	© 2021, Zachary Berry
==================================================================================================*/

/*
*	Header Includes
*/
#include "Hyperion/Renderer/DirectX12/DX12Graphics.h"

namespace Hyperion
{
	/*
	*	DX12Graphics::DX12Graphics()
	*/
	DX12Graphics::DX12Graphics()
		: m_GraphicsMemory( 0 ), m_GraphicsCardName( "Unknowm" ), m_Resolution(), m_OutputWindow( nullptr ), m_BackBufferIndex( 0 ), m_FenceValue( 0 )
	{

	}

	/*
	*	DX12Graphics::~DX12Graphics()
	*/
	DX12Graphics::~DX12Graphics()
	{
		Shutdown();
	}

	/*
	*	DX12Graphics::Initialize( voidWindowPointer )
	*/
	bool DX12Graphics::Initialize( void* inWindow )
	{
		HYPERION_VERIFY( inWindow != nullptr, "[DX12] Window handle is null" );
		HYPERION_VERIFY( m_Device == nullptr, "[DX12] Already initialized" );
		Console::WriteLine( "[DX12] Initializing..." );

		auto window = static_cast< HWND >( inWindow );

		// Initialize our resources
		if( !InitializeResources( window, m_Resolution ) )
		{
			return false;
		}

		// TODO: Other initialization

		Console::WriteLine( "[DX12] Resources initialized!" );
		return true;
	}

	/*
	*	DX12Graphics::Shutdown()
	*/
	void DX12Graphics::Shutdown()
	{
		ShutdownResources();
	}

	/*
	*	DX12Graphics::InitializeResources( windowPointer, screenResolution )
	*/
	bool DX12Graphics::InitializeResources( HWND inWindow, const ScreenResolution& inRes )
	{
		HYPERION_VERIFY( inRes.Width > 0 && inRes.Height > 0, "[DX12] Resolution invalid" );

		// Wrap this code in a try block, so if an exception is thrown, we can still exit gracefully
		try
		{
			// Create the DXGI factory we need
			IDXGIFactory4* factoryPtr = nullptr;
			if( FAILED( CreateDXGIFactory1( __uuidof( IDXGIFactory4 ), (void**) &factoryPtr ) ) || factoryPtr == nullptr )
			{
				throw std::exception( "Failed to create factory" );
			}

			std::unique_ptr< IDXGIFactory4 > factory( factoryPtr );

			// Get video adapter
			IDXGIAdapter* adapterPtr = nullptr;
			if( FAILED( factory->EnumAdapters( 0, &adapterPtr ) ) || adapterPtr == nullptr )
			{
				throw std::exception( "Failed to enumerate video adapters" );
			}

			std::unique_ptr< IDXGIAdapter > adapter( adapterPtr );

			// Get the output
			IDXGIOutput* outputPtr = nullptr;
			if( FAILED( adapter->EnumOutputs( 0, &outputPtr ) ) || outputPtr == nullptr )
			{
				throw std::exception( "Failed to enumerate video output" );
			}

			std::unique_ptr< IDXGIOutput > output( outputPtr );

			// Now we need to get a list of display modes were able to use
			uint32 numModes = 0;
			if( FAILED( output->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr ) ) || numModes == 0 )
			{
				throw std::exception( "Failed to find display mode with correct color format" );
			}

			std::vector< DXGI_MODE_DESC > displayModes;
			displayModes.resize( numModes );

			if( FAILED( output->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModes.data() ) ) )
			{
				throw std::exception( "Failed to find display mode with correct color format" );
			}

			// Now, we need to find the resolution that matches the resolution we were supplied
			bool bResolutionSupported = false;
			DXGI_MODE_DESC selectedMode {};

			for( auto it = displayModes.begin(); it != displayModes.end(); it++ )
			{
				if( it->Width == inRes.Width && it->Height == inRes.Height )
				{
					bResolutionSupported = true;
					selectedMode = *it;
					break;
				}
			}

			if( !bResolutionSupported )
			{
				throw std::exception( "Selected resolution was not supported!" );
			}

			// Store the current resolution were using
			m_Resolution.Width		= selectedMode.Width;
			m_Resolution.Height		= selectedMode.Height;

			// Get information about the graphics card
			DXGI_ADAPTER_DESC adapterDesc {};
			if( FAILED( adapter->GetDesc( &adapterDesc ) ) )
			{
				throw std::exception( "Failed to get graphics adapter description" );
			}

			m_GraphicsMemory	= adapterDesc.DedicatedVideoMemory / 1024 / 1024;
			m_GraphicsCardName	= String( std::wstring( adapterDesc.Description ), StringEncoding::UTF16 );
			
			// Resize the window to match our resolution and then center it
			RECT windowSize = { 0, 0, (long) m_Resolution.Width, (long) m_Resolution.Height };
			if( !AdjustWindowRectEx( &windowSize, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW ) )
			{
				throw std::exception( "Failed to resize window to match resolution" );
			}

			SetWindowPos( inWindow, HWND_TOP, 0, 0, windowSize.right - windowSize.left, windowSize.bottom - windowSize.top, SWP_NOMOVE );

			// Create DX12 device
			if( FAILED( D3D12CreateDevice( adapter.get(), D3D_FEATURE_LEVEL_12_1, __uuidof( ID3D12Device4 ), (void**)m_Device.ReleaseAndGetAddressOf() ) ) || m_Device == nullptr )
			{
				throw std::exception( "Failed to create d3d12 device" );
			}

			// Release resources we no longer need
			adapter->Release();
			adapter.reset();

			output->Release();
			output.reset();
			
			// Create the swap chain
			DXGI_SWAP_CHAIN_DESC chainDesc {};

			chainDesc.BufferCount			= 2;
			chainDesc.BufferDesc.Format		= DXGI_FORMAT_R8G8B8A8_UNORM;
			chainDesc.BufferDesc.Width		= m_Resolution.Width;
			chainDesc.BufferDesc.Height		= m_Resolution.Height;
			chainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
			chainDesc.OutputWindow			= m_OutputWindow;
			chainDesc.SampleDesc.Count		= 1;
			chainDesc.SampleDesc.Quality	= 0;
			chainDesc.Windowed				= !m_Resolution.FullScreen;
			chainDesc.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD;
			chainDesc.Flags					= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

			if( m_bVSync )
			{
				chainDesc.BufferDesc.RefreshRate.Numerator		= 0;
				chainDesc.BufferDesc.RefreshRate.Denominator	= 1;
			}

			IDXGISwapChain* swapChainPtr = nullptr;
			if( FAILED( factory->CreateSwapChain( m_Device.Get(), &chainDesc, &swapChainPtr ) ) || swapChainPtr == nullptr )
			{
				throw std::exception( "Failed to create swap chain" );
			}

			if( FAILED( swapChainPtr->QueryInterface< IDXGISwapChain3 >( m_SwapChain.ReleaseAndGetAddressOf() ) ) || m_SwapChain == nullptr )
			{
				swapChainPtr = nullptr;
				throw std::exception( "Failed to query swap chain interface" );
			}

			swapChainPtr = nullptr;

			// Query for the debug interface
		#ifdef HYPERION_DEBUG_RENDERER
			if( FAILED( m_Device->QueryInterface< ID3D12Debug >( m_Debug.ReleaseAndGetAddressOf() ) ) || m_Debug == nullptr )
			{
				throw std::exception( "Failed to query debug interface" );
			}
		#endif

			// Lets release the factory, we dont need it anymore
			factory->Release();
			factory.reset();

			// Create the command queue
			D3D12_COMMAND_QUEUE_DESC cmdQueueDesc {};

			cmdQueueDesc.Type		= D3D12_COMMAND_LIST_TYPE_DIRECT;
			cmdQueueDesc.Priority	= D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			cmdQueueDesc.Flags		= D3D12_COMMAND_QUEUE_FLAG_NONE;
			cmdQueueDesc.NodeMask	= 0;

			if( FAILED( m_Device->CreateCommandQueue( &cmdQueueDesc, __uuidof( ID3D12CommandQueue ), (void**) m_CommandQueue.ReleaseAndGetAddressOf() ) ) || m_CommandQueue == nullptr )
			{
				throw std::exception( "Failed to create command queue" );
			}

			// Create heap description for both swap chain back buffers
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc {};

			heapDesc.NumDescriptors		= 2;
			heapDesc.Type				= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			heapDesc.Flags				= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

			if( FAILED( m_Device->CreateDescriptorHeap( &heapDesc, __uuidof( ID3D12DescriptorHeap ), (void**) m_BackBufferTargetHeap.ReleaseAndGetAddressOf() ) ) || m_BackBufferTargetHeap == nullptr )
			{
				throw std::exception( "Failed to create back buffer descriptor heaps" );
			}

			// Get memory location and size for the two back buffers
			auto backBufferDescriptor		= m_BackBufferTargetHeap->GetCPUDescriptorHandleForHeapStart();
			auto backBufferDescriptorSize	= m_Device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );

			// Get pointer to the two back buffer resources from the swap chain
			if( FAILED( m_SwapChain->GetBuffer( 0, __uuidof( ID3D12Resource ), (void**) m_BackBufferFront.ReleaseAndGetAddressOf() ) ) || m_BackBufferFront == nullptr ||
				FAILED( m_SwapChain->GetBuffer( 1, __uuidof( ID3D12Resource ), (void**) m_BackBufferBack.ReleaseAndGetAddressOf() ) ) || m_BackBufferBack == nullptr )
			{
				throw std::exception( "Failed to get back buffer resources" );
			}

			// Create the first render target view
			m_Device->CreateRenderTargetView( m_BackBufferFront.Get(), nullptr, backBufferDescriptor );

			// Increment heap pointer
			backBufferDescriptor.ptr += backBufferDescriptorSize;

			// Create second render target view
			m_Device->CreateRenderTargetView( m_BackBufferBack.Get(), nullptr, backBufferDescriptor );

			// Get the current back buffer index
			m_BackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

			// Create command allocator
			if( FAILED( m_Device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof( ID3D12CommandAllocator ), (void**) m_CommandAllocator.ReleaseAndGetAddressOf() ) ) || m_CommandAllocator == nullptr )
			{
				throw std::exception( "Failed to create command allocator" );
			}

			// Create command list
			if( FAILED( m_Device->CreateCommandList1( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, __uuidof( ID3D12GraphicsCommandList ), (void**) m_CommandList.ReleaseAndGetAddressOf() ) ) )
			{
				throw std::exception( "Failed to create command list" );
			}

			// Setup the render fence, to sync GPU and CPU work per frame
			if( FAILED( m_Device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, __uuidof( ID3D12Fence ), (void**) m_RenderFence.ReleaseAndGetAddressOf() ) ) || m_RenderFence == nullptr )
			{
				throw std::exception( "Failed to create render fence" );
			}

			m_RenderFenceEvent = CreateEventEx( NULL, FALSE, FALSE, EVENT_ALL_ACCESS );
			if( m_RenderFenceEvent == NULL )
			{
				throw std::exception( "Failed to create render fence event" );
			}

			m_FenceValue = 1;
		}
		catch( std::exception ex )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to intiialize renderer resources (", ex.what(), ")" );
			ShutdownResources();
		}

		// Store pointer to window handle
		m_OutputWindow = inWindow;


		return true;
	}

	/*
	*	DX12Graphics::ShutdownResources()
	*/
	void DX12Graphics::ShutdownResources()
	{
		m_RenderFence.Reset();
		m_CommandList.Reset();
		m_CommandAllocator.Reset();
		m_BackBufferBack.Reset();
		m_BackBufferFront.Reset();
		m_BackBufferTargetHeap.Reset();
		m_Debug.Reset();
		m_SwapChain.Reset();
		m_CommandQueue.Reset();
		m_Device.Reset();

		m_OutputWindow = nullptr;
	}

}