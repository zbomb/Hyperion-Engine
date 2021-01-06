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

	// Forward Declarations
	class DirectX11RenderTarget;
	class DirectX11Texture2D;

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
		ComPtr< ID3D11Texture2D > m_DepthStencilBuffer;
		ComPtr< ID3D11RasterizerState > m_RasterizerState;
		ComPtr< ID3D11DepthStencilView > m_DepthStencilView;
		ComPtr< ID3D11DepthStencilState > m_DepthStencilState;
		ComPtr< ID3D11DepthStencilState > m_DepthDisabledState;
		ComPtr< ID3D11BlendState > m_BlendState;
		ComPtr< ID3D11BlendState > m_BlendDisabledState;

		std::shared_ptr< DirectX11RenderTarget > m_RenderTarget;
		std::shared_ptr< DirectX11Texture2D > m_BackBuffer;

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

		bool Initialize( void* pWindow ) override;
		void Shutdown() override;

		bool IsRunning() const override;

		void BeginFrame() override;
		void EndFrame() override;

		void EnableAlphaBlending() override;
		void DisableAlphaBlending() override;
		bool IsAlphaBlendingEnabled() override;

		void EnableZBuffer() override;
		void DisableZBuffer() override;
		bool IsZBufferEnabled() override;

		std::shared_ptr< RRenderTarget > GetRenderTarget() override;
		std::shared_ptr< RTexture2D > GetBackBuffer() override;


		std::vector< ScreenResolution > GetAvailableResolutions() override;

		inline bool AllowAsyncTextureCreation() const override { return true; }

		std::shared_ptr< RBuffer > CreateBuffer( const BufferParameters& inParams ) override;
		std::shared_ptr< RBuffer > CreateBuffer( BufferType ty = BufferType::Vertex ) override;
		
		// Texture Creation
		std::shared_ptr< RTexture1D > CreateTexture1D( const TextureParameters& ) final;
		std::shared_ptr< RTexture2D > CreateTexture2D( const TextureParameters& ) final;
		std::shared_ptr< RTexture3D > CreateTexture3D( const TextureParameters& ) final;
		std::shared_ptr< RTexture1D > CreateTexture1D() final;
		std::shared_ptr< RTexture2D > CreateTexture2D() final;
		std::shared_ptr< RTexture3D > CreateTexture3D() final;

		// Texture Copying
		bool CopyTexture1D( std::shared_ptr< RTexture1D >& inSource, std::shared_ptr< RTexture1D >& inDest ) final;
		bool CopyTexture2D( std::shared_ptr< RTexture2D >& inSource, std::shared_ptr< RTexture2D >& inDest ) final;
		bool CopyTexture3D( std::shared_ptr< RTexture3D >& inSource, std::shared_ptr< RTexture3D >& inDest ) final;

		bool CopyTexture1DRegion( std::shared_ptr< RTexture1D >& inSource, std::shared_ptr< RTexture1D >& inDest,
										  uint32 sourceX, uint32 inWidth, uint32 destX, uint8 sourceMip, uint8 targetMip ) final;
		bool CopyTexture2DRegion( std::shared_ptr< RTexture2D >& inSource, std::shared_ptr< RTexture2D >& inDest, uint32 sourceX, uint32 sourceY,
										  uint32 inWidth, uint32 inHeight, uint32 destX, uint32 destY, uint8 sourceMip, uint8 targetMip ) final;
		bool CopyTexture3DRegion( std::shared_ptr< RTexture3D >& inSource, std::shared_ptr< RTexture3D >& inDest, uint32 sourceX, uint32 sourceY, uint32 sourceZ,
										  uint32 inWidth, uint32 inHeight, uint32 inDepth, uint32 destX, uint32 destY, uint32 destZ, uint8 sourceMip, uint8 targetMip ) final;

		bool CopyTexture1DMip( std::shared_ptr< RTexture1D >& inSource, std::shared_ptr< RTexture1D >& inDest, uint8 sourceMip, uint8 destMip ) final;
		bool CopyTexture2DMip( std::shared_ptr< RTexture2D >& inSource, std::shared_ptr< RTexture2D >& inDest, uint8 sourceMip, uint8 destMip ) final;
		bool CopyTexture3DMip( std::shared_ptr< RTexture3D >& inSource, std::shared_ptr< RTexture3D >& inDest, uint8 sourceMip, uint8 destMip ) final;


		std::shared_ptr< RRenderTarget > CreateRenderTarget( std::shared_ptr< RTexture2D > inSource ) override;
	};

}


#endif