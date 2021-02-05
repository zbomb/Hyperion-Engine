/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/Shaders/DX11GBufferPixelShader.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resources/RShader.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


namespace Hyperion
{

	class DX11GBufferPixelShader : public RPixelShader
	{

	private:

		Microsoft::WRL::ComPtr< ID3D11PixelShader > m_PixelShader;
		Microsoft::WRL::ComPtr< ID3D11SamplerState > m_WrapSampler;
		Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_Context;

	public:

		DX11GBufferPixelShader();
		~DX11GBufferPixelShader();

		bool Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext );

		void Shutdown() final;
		bool IsValid() const final;

		bool UploadPrimitiveParameters( const Matrix& inWorldMatrix, const RMaterial& inMaterial ) final;

		inline bool UploadStaticParameters( Renderer& inRenderer, uint32 inFlags ) final	{ return true; }
		inline bool UploadLightBuffer( RLightBuffer& inBuffer ) final		{ return true; }
		inline bool UploadViewClusters( RViewClusters& inClusters ) final	{ return true; }
		inline bool UploadGBuffer( GBuffer& inBuffer ) final				{ return true; } // The G-Buffer is the render output, not an input

		bool Attach() final;
		void Detach() final;

		inline ID3D11PixelShader* GetDX11Shader() const { return m_PixelShader ? m_PixelShader.Get() : nullptr; }
	};

}