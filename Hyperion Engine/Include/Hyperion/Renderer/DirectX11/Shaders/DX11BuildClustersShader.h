/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/Shaders/DX11BuildClustersShader.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resources/RShader.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


namespace Hyperion
{

	class DX11BuildClustersShader : public RComputeShader
	{

	private:

		Microsoft::WRL::ComPtr< ID3D11ComputeShader > m_Shader;
		Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_Context;
		Microsoft::WRL::ComPtr< ID3D11Buffer > m_ConstantBuffer;

		#pragma pack( push, 1 )
		struct ConstantBufferType
		{
			float ScreenWidth;
			float ScreenHeight;
			float ScreenNear;
			float ScreenFar;

			uint32 ShaderMode;
			uint32 _pad_1;
			uint32 _pad_2;
			uint32 _pad_3;
			
			DirectX::XMMATRIX InvProjectionMatrix;
		};
		#pragma pack( pop )

		bool m_bAttached;

	public:

		DX11BuildClustersShader();
		~DX11BuildClustersShader();

		bool Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext );

		void Shutdown() final;
		bool IsValid() const final;

		inline bool UploadLightBuffer( RLightBuffer& inLights ) final		{ return true; }
		inline bool UploadGBuffer( GBuffer& inGBuffer ) final				{ return true; }

		bool UploadStaticParameters( Renderer& inRenderer, uint32 inFlags ) final;
		bool UploadViewClusters( RViewClusters& inClusters ) final;

		inline bool RequiresGBuffer() const { return false; }

		bool Attach() final;
		void Detach() final;

		bool Dispatch() final;

	};

}