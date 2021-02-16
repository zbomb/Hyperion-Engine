/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DX12/DX12Core.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

/*
*	Headers
*/
#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/DX12/DX12.h"
#include "Hyperion/Renderer/Misc.h"


namespace Hyperion
{

	class DX12Core
	{

	private:

		template< typename _T >
		using ComPtr = Microsoft::WRL::ComPtr< _T >;

		/*
		*	Data Members
		*/
		HWND m_OutputSurface;
		ScreenResolution m_Resolution;
		bool m_bVSync;
		String m_GraphicsCardName;
		uint32 m_GraphicsMemory;
		uint32 m_BackBufferIndex;

		/*
		*	DX12 Resources
		*/
		ComPtr< ID3D12Device4 > m_Device;
		ComPtr< ID3D12CommandQueue > m_CommandQueue;
		ComPtr< IDXGISwapChain3 > m_SwapChain;
		ComPtr< ID3D12Debug > m_Debug;
		ComPtr< ID3D12DescriptorHeap > m_BackBufferTargetHeap;
		ComPtr< ID3D12Resource > m_BackBufferFront;
		ComPtr< ID3D12Resource > m_BackBufferBack;
		ComPtr< ID3D12CommandAllocator > m_CommandAllocator;
		ComPtr< ID3D12GraphicsCommandList > m_CommandList;
		ComPtr< ID3D12Fence > m_RenderFence;

		HANDLE m_RenderFenceEvent;
		uint32 m_FenceValue;

		/*
		*	Internal Functions
		*/
		bool InitializeResources( HWND inWindow, const ScreenResolution& inRes, bool bVSync );
		void ShutdownResources();

	public:

		DX12Core();
		~DX12Core();

		bool Initialize( HWND inWindow, const ScreenResolution& inResolution, bool bVSync );
		void Shutdown();

		bool UpdateResolution( const ScreenResolution& inRes );
		bool UpdateVSync( bool bVSync );
		bool IsResolutionValid( const ScreenResolution& inRes );

		inline ScreenResolution GetResolution() const { return m_Resolution; }
		inline bool GetVSync() const { return m_bVSync; }

		D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferTarget();
		void FlipSwapChain();
	};

}