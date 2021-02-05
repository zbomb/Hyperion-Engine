/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/Shaders/DX11CullLightsShader.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resources/RShader.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


namespace Hyperion
{

	class DX11CullLightsShader : public RComputeShader
	{

	private:

		Microsoft::WRL::ComPtr< ID3D11ComputeShader > m_Shader;
		Microsoft::WRL::ComPtr< ID3D11Buffer > m_ConstantBuffer;
		Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_Context;

		bool m_bAttached;

		#pragma pack( push, 1 )
		struct ConstantBufferType
		{
			uint32 LightCount;
			uint32 _pad_1;
			uint32 _pad_2;
			uint32 _pad_3;

			DirectX::XMMATRIX ViewMatrix;
		};
		#pragma pack( pop )

	public:

		DX11CullLightsShader();
		~DX11CullLightsShader();

		bool Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext );

		void Shutdown() final;
		bool IsValid() const final;

		bool UploadLightBuffer( RLightBuffer& inLights ) final;
		bool UploadStaticParameters( Renderer& inRenderer, uint32 inFlags ) final;
		bool UploadViewClusters( RViewClusters& inClusters ) final;

		inline bool UploadGBuffer( GBuffer& inGBuffer ) final { return true; }
		inline bool RequiresGBuffer() const { return false; }

		bool Attach() final;
		void Detach() final;

		bool Dispatch() final;

	};

}