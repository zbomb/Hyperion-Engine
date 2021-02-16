/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/Shaders/DX11FXAAShader.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resources/RShader.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


namespace Hyperion
{

	class DX11FXAAShader : public RPostProcessShader
	{

	private:

		Microsoft::WRL::ComPtr< ID3D11PixelShader > m_Shader;
		Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_Context;
		Microsoft::WRL::ComPtr< ID3D11SamplerState > m_Sampler;
		Microsoft::WRL::ComPtr< ID3D11Buffer > m_ConstantBuffer;

		#pragma pack( push, 1 )
		struct ConstantBufferType
		{
			float ScreenWidth;
			float ScreenHeight;
			float _pad_1;
			float _pad_2;
		};
		#pragma pack( pop )

	public:

		DX11FXAAShader();
		~DX11FXAAShader();

		bool Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext );
		void Shutdown() final;
		bool IsValid() const final;

		bool UploadStaticParameters( Renderer& inRenderer, uint32 inFlags ) final;
		bool Attach( const std::shared_ptr< RTexture2D >& inSource ) final;
		void Detach() final;

	};

}
