/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/DirectX11Renderer.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once
#include "Hyperion/Hyperion.h"

#ifdef HYPERION_SUPPORT_DIRECTX

// Hyperion Includes
#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


// Forward Declarations
namespace DirectX
{
	class EffectFactory;
	class CommonStates;
}


namespace Hyperion
{
	// Constants
	constexpr float SCREEN_NEAR		= 0.1f;
	constexpr float SCREEN_FAR		= 1000.f;

	class DirectX11Renderer : public Renderer
	{
		template< typename T >
		using ComPtr = Microsoft::WRL::ComPtr< T >;

	public:

		DirectX11Renderer();
		~DirectX11Renderer();

	protected:
		
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
		
		//std::unique_ptr< DirectX::EffectFactory > m_EffectFactory;
		//std::unique_ptr< DirectX::CommonStates > m_CommonStates;

		// Parameters
		HWND m_RenderTarget;
		bool m_VSync;
		ScreenResolution m_Resolution;

		uint32 m_VideoCardMemory;
		String m_VideoCardDescription;

		DirectX::XMFLOAT4X4 m_ProjectionMatrix, m_WorldMatrix, m_OrthoMatrix, m_ScreenMatrix;

		virtual bool SetScreenResolution( const ScreenResolution& inResolution ) override;
		inline virtual ScreenResolution GetScreenResolution() override { return m_Resolution; }
		virtual std::vector< ScreenResolution > GetAvailableResolutions() override;

		virtual bool SetRenderTarget( HWND inTarget ) override;
		inline virtual HWND GetRenderTarget() override { return m_RenderTarget; }

		virtual void SetVSync( bool inVSync ) override;
		inline virtual bool GetVSyncEnabled() override { return m_VSync; }

		inline virtual uint32 GetVideoCardMemory() override { return m_VideoCardMemory; }
		inline virtual String GetVideoCardDescription() override { return m_VideoCardDescription; }

		virtual bool Init() override;
		virtual void Frame() override;
		virtual void Shutdown() override;

		void GenerateMatricies( const ScreenResolution& inRes, float inFOV, float inNear, float inFar );

		bool InitializeResources( HWND renderTarget, ScreenResolution& inOutRes );
		void ShutdownResources();

		void EnableAlphaBlending();
		void DisableAlphaBlending();
		bool IsAlphaBlendingEnabled();

		void EnableZBuffer();
		void DisableZBuffer();
		bool IsZBufferEnabled();
	};

}

#endif