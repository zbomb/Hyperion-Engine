/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/Shaders/DirectX11Compute.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resource/Shader.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"
#include "Hyperion/Renderer/DirectX11/DirectX11ViewClusters.h"


namespace Hyperion
{

	struct ClusterShaderScreenBuffer
	{
		DirectX::XMMATRIX InvProjection;
		DirectX::XMUINT3 ClusterCount;
		uint32 _pad_1;
		DirectX::XMUINT2 ClusterSize;
		DirectX::XMUINT2 ScreenSize;
		float NearZ;
		float FarZ;
		float _pad_2;
		float _pad_3;
	};


	class DirectX11BuildClusterShader : public RBuildClusterShader
	{

	private:

		Microsoft::WRL::ComPtr< ID3D11ComputeShader > m_Shader;
		String m_ShaderPath;
		ID3D11DeviceContext* m_Context;

		const uint32 m_ClusterCountX;
		const uint32 m_ClusterCountY;
		const uint32 m_ClusterCountZ;

		float m_ClusterSizeX;
		float m_ClusterSizeY;

		Microsoft::WRL::ComPtr< ID3D11Buffer > m_ScreenInfoBuffer;

	public:

		DirectX11BuildClusterShader() = delete;
		DirectX11BuildClusterShader( const String& inShaderPath );
		~DirectX11BuildClusterShader();

		bool Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext );

		inline bool IsValid() const override { return m_Shader ? true : false; }
		void Shutdown() override;

		bool UploadViewInfo( const Matrix& inProjectionMatrix, const ScreenResolution& inResolution, float inScreenNear, float inScreenFar ) override;

		void Attach() override;
		void Detach() override;

		bool Dispatch( const std::shared_ptr< RViewClusters >& outClusters ) override;

	};


	class DirectX11CompressClustersShader : public RCompressClustersShader
	{

	private:

		String m_ShaderPath;
		Microsoft::WRL::ComPtr< ID3D11ComputeShader > m_Shader;
		ID3D11DeviceContext* m_Context;

	public:

		DirectX11CompressClustersShader() = delete;
		DirectX11CompressClustersShader( const String& inShaderPath );
		~DirectX11CompressClustersShader();

		bool Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext );

		inline bool IsValid() const override { return m_Shader ? true : false; }
		void Shutdown() override;

		void Attach() override;
		void Detach() override;
		bool Dispatch( const std::shared_ptr< RViewClusters >& outClusters ) override;
	};

}