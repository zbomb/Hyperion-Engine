/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/DirectX11Graphics.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"

#ifdef HYPERION_SUPPORT_DIRECTX

#include "Hyperion/Renderer/IGraphics.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"
#include "Hyperion/Core/String.h"



namespace Hyperion
{

	class DirectX11Graphics : public IGraphics
	{

		template< typename _Ty >
		using ComPtr = Microsoft::WRL::ComPtr< _Ty >;

	private:

		/*
			Data Members
		*/
		HWND m_Output;
		ScreenResolution m_Resolution;
		bool m_bVSync;
		bool m_bRunning;

		uint32 m_GraphicsMemory;
		String m_GraphicsDevice;

		/*
			Resource Pointers
		*/
		ComPtr< IDXGISwapChain > m_SwapChain;
		ComPtr< ID3D11Device > m_Device;
		ComPtr< ID3D11DeviceContext > m_DeviceContext;
		ComPtr< ID3D11RenderTargetView > m_RenderTargetView;
		ComPtr< ID3D11Texture2D > m_DepthStencilBuffer;
		ComPtr< ID3D11RasterizerState > m_RasterizerState;
		ComPtr< ID3D11DepthStencilView > m_DepthStencilView;
		ComPtr< ID3D11DepthStencilState > m_DepthStencilState;
		ComPtr< ID3D11DepthStencilState > m_DepthDisabledState;
		ComPtr< ID3D11BlendState > m_BlendState;
		ComPtr< ID3D11BlendState > m_BlendDisabledState;
		ComPtr< ID3D11Texture2D > m_BackBuffer;

		/*
			Matricies
		*/
		DirectX::XMFLOAT4X4 m_ProjectionMatrix, m_WorldMatrix, m_OrthoMatrix, m_ScreenMatrix;

		/*
			Helper Functions
		*/
		bool InitializeResources( HWND Target, ScreenResolution& Resolution );
		void ShutdownResources();
		void GenerateMatricies( const ScreenResolution& inRes, float inFOV, float inNear, float inFar );


	public:

		DirectX11Graphics();
		~DirectX11Graphics();

		bool SetResolution( const ScreenResolution& inResolution ) override;
		void SetVSync( bool bVSync ) override;

		bool Initialize( const IRenderOutput& Output ) override;
		void Shutdown() override;

		void BeginFrame() override;
		void EndFrame() override;

		void EnableAlphaBlending() override;
		void DisableAlphaBlending() override;
		bool IsAlphaBlendingEnabled() override;

		void EnableZBuffer() override;
		void DisableZBuffer() override;
		bool IsZBufferEnabled() override;

		std::vector< ScreenResolution > GetAvailableResolutions() override;


	};

}


#endif