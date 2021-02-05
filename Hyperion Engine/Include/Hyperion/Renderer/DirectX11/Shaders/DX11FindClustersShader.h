/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/Shaders/DX11FindClustersShader.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resources/RShader.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


namespace Hyperion
{


	class DX11FindClustersShader : public RComputeShader
	{

	private:

		Microsoft::WRL::ComPtr< ID3D11ComputeShader > m_Shader;
		Microsoft::WRL::ComPtr< ID3D11Buffer > m_ConstantBuffer;
		Microsoft::WRL::ComPtr< ID3D11SamplerState > m_Sampler;
		Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_Context;

		#pragma pack( push, 1 )
		struct ConstantBufferType
		{
			float ScreenWidth;
			float ScreenHeight;
			float DepthSliceA;
			float DepthSliceB;
			float ScreenNear;
			float ScreenFar;
			float _pad_1;
			float _pad_2;
		};
		#pragma pack( pop )

		bool m_bAttached;
		uint32 m_ScreenWidth;
		uint32 m_ScreenHeight;

	public:

		DX11FindClustersShader();
		~DX11FindClustersShader();

		bool Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext );

		void Shutdown() final;
		bool IsValid() const final;

		inline bool UploadLightBuffer( RLightBuffer& inLights ) final { return true; }

		bool UploadStaticParameters( Renderer& inRenderer, uint32 inFlags ) final;
		bool UploadGBuffer( GBuffer& inGBuffer ) final;
		bool UploadViewClusters( RViewClusters& inClusters ) final;

		inline bool RequiresGBuffer() const { return true; }

		bool Attach() final;
		void Detach() final;

		bool Dispatch() final;

	};

}