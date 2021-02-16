/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/Shaders/DX11ForwardPixelShader.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resources/RShader.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


namespace Hyperion
{

	class DX11ForwardPixelShader : public RPixelShader
	{

	private:

		Microsoft::WRL::ComPtr< ID3D11PixelShader > m_PixelShader;
		Microsoft::WRL::ComPtr< ID3D11Buffer > m_RenderInfoBuffer;
		Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_Context;
		Microsoft::WRL::ComPtr< ID3D11SamplerState > m_Sampler;

		#pragma pack( push, 1 )
		struct RenderInfoBuffer
		{
			DirectX::XMFLOAT3 CameraPosition;
			float _pad_vb_1;

			float ProjectionTermA;
			float ProjectionTermB;
			float ScreenNear;
			float ScreenFar;

			float ScreenWidth;
			float ScreenHeight;
			float DepthSliceTermA;
			float DepthSliceTermB;

			DirectX::XMFLOAT3 AmbientLightColor;
			float AmbientLightIntensity;

			DirectX::XMMATRIX InverseViewMatrix;
		};
		#pragma pack( pop )

	public:

		DX11ForwardPixelShader();
		~DX11ForwardPixelShader();

		bool Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext );
		void Shutdown() final;

		bool IsValid() const final;

		bool UploadStaticParameters( Renderer& inRenderer, uint32 inFlags ) final;
		bool UploadLightBuffer( RLightBuffer& inBuffer ) final;
		bool UploadViewClusters( RViewClusters& inClusters ) final;
		bool UploadBatchMaterial( const RMaterial& inMaterial ) final;

		inline bool UploadBatchTransforms( const std::vector< Matrix >& inMatricies ) final { return true; }
		inline bool UploadGBuffer( GBuffer& inBuffer ) final { return true; }

		bool Attach() final;
		void Detach() final;

		inline ID3D11PixelShader* GetDX11Shader() const { return m_PixelShader ? m_PixelShader.Get() : nullptr; }
	};

}