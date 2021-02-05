/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/DirectX11ViewClusters.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once


#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/ViewClusters.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


namespace Hyperion
{

	struct ClusterCellInfo
	{
		DirectX::XMFLOAT3 MinimumAABB;
		float _pad_1;
		DirectX::XMFLOAT3 MaximumAABB;
		uint32 bActive;
	};

	struct ClusterLightAssignment
	{
		UINT lightOffset;
		UINT lightCount;
	};


	class DirectX11ViewClusters : public RViewClusters
	{

	private:

		// Cluster Info Buffer
		Microsoft::WRL::ComPtr< ID3D11Buffer > m_ClusterInfoBuffer;
		Microsoft::WRL::ComPtr< ID3D11UnorderedAccessView > m_ClusterInfoUAV;
		Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > m_ClusterInfoSRV;

		// Cluster Light Index (List of offsets & lengths for the lightIdBuffer)
		Microsoft::WRL::ComPtr< ID3D11Texture3D > m_ClusterLightIndex;
		Microsoft::WRL::ComPtr< ID3D11UnorderedAccessView > m_ClusterLightUAV;
		Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > m_ClusterLightSRV;

		Microsoft::WRL::ComPtr< ID3D11Buffer > m_LightIdentifierList;
		Microsoft::WRL::ComPtr< ID3D11UnorderedAccessView > m_LightIdentifierUAV;
		Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > m_LightIdentifierSRV;

		bool m_bDirty;

	public:

		DirectX11ViewClusters()
			: m_bDirty( true )
		{
		}


		~DirectX11ViewClusters()
		{
			Shutdown();
		}


		bool Initialize( ID3D11Device* inDevice )
		{
			HYPERION_VERIFY( inDevice, "[DX11] Device was null!" );

			// Create the cluster info buffer
			D3D11_BUFFER_DESC clusterInfoDesc;
			ZeroMemory( &clusterInfoDesc, sizeof( clusterInfoDesc ) );

			clusterInfoDesc.Usage					= D3D11_USAGE_DEFAULT;
			clusterInfoDesc.ByteWidth				= sizeof( ClusterCellInfo ) * RENDERER_CLUSTER_COUNT_X * RENDERER_CLUSTER_COUNT_Y * RENDERER_CLUSTER_COUNT_Z;
			clusterInfoDesc.BindFlags				= D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			clusterInfoDesc.CPUAccessFlags			= 0;
			clusterInfoDesc.MiscFlags				= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			clusterInfoDesc.StructureByteStride		= sizeof( ClusterCellInfo );

			D3D11_TEXTURE3D_DESC lightIndexDesc;
			ZeroMemory( &lightIndexDesc, sizeof( lightIndexDesc ) );

			lightIndexDesc.Format			= DXGI_FORMAT_R32G32_UINT;
			lightIndexDesc.MipLevels		= 1;
			lightIndexDesc.Width			= RENDERER_CLUSTER_COUNT_X;
			lightIndexDesc.Height			= RENDERER_CLUSTER_COUNT_Y;
			lightIndexDesc.Depth			= RENDERER_CLUSTER_COUNT_Z;
			lightIndexDesc.BindFlags		= D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			lightIndexDesc.CPUAccessFlags	= 0;
			lightIndexDesc.MiscFlags		= 0;
			lightIndexDesc.Usage			= D3D11_USAGE_DEFAULT;

			D3D11_BUFFER_DESC lightIdDesc;
			ZeroMemory( &lightIdDesc, sizeof( lightIdDesc ) );

			lightIdDesc.Usage					= D3D11_USAGE_DEFAULT;
			lightIdDesc.BindFlags				= D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			lightIdDesc.ByteWidth				= sizeof( UINT ) * RENDERER_CLUSTER_COUNT_X * RENDERER_CLUSTER_COUNT_Y * RENDERER_CLUSTER_COUNT_Z * RENDERER_CLUSTER_MAX_LIGHTS;
			lightIdDesc.StructureByteStride		= sizeof( UINT );
			lightIdDesc.CPUAccessFlags			= 0;
			lightIdDesc.MiscFlags				= 0;
			


			// TODO: Set initial values to zero?
			if( FAILED( inDevice->CreateBuffer( &clusterInfoDesc, NULL, m_ClusterInfoBuffer.ReleaseAndGetAddressOf() ) ) ||
				FAILED( inDevice->CreateTexture3D( &lightIndexDesc, NULL, m_ClusterLightIndex.ReleaseAndGetAddressOf() ) ) ||
				FAILED( inDevice->CreateBuffer( &lightIdDesc, NULL, m_LightIdentifierList.ReleaseAndGetAddressOf() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create view cluster structure, couldnt create buffers" );
				Shutdown();
				return false;
			}

			// Create the two views of the buffer
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			ZeroMemory( &uavDesc, sizeof( uavDesc ) );

			uavDesc.Buffer.FirstElement		= 0;
			uavDesc.Buffer.Flags			= 0;
			uavDesc.Buffer.NumElements		= RENDERER_CLUSTER_COUNT_X * RENDERER_CLUSTER_COUNT_Y * RENDERER_CLUSTER_COUNT_Z;
			uavDesc.Format					= DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension			= D3D11_UAV_DIMENSION_BUFFER;

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory( &srvDesc, sizeof( srvDesc ) );

			srvDesc.Format					= DXGI_FORMAT_UNKNOWN;
			srvDesc.Buffer.ElementOffset	= 0;
			srvDesc.Buffer.ElementWidth		= sizeof( ClusterCellInfo );
			srvDesc.Buffer.FirstElement		= 0;
			srvDesc.Buffer.NumElements		= uavDesc.Buffer.NumElements;
			srvDesc.ViewDimension			= D3D11_SRV_DIMENSION_BUFFER;

			D3D11_UNORDERED_ACCESS_VIEW_DESC lightIndexUavDesc;
			ZeroMemory( &lightIndexUavDesc, sizeof( lightIndexUavDesc ) );

			lightIndexUavDesc.Format					= DXGI_FORMAT_R32G32_UINT;
			lightIndexUavDesc.ViewDimension				= D3D11_UAV_DIMENSION_TEXTURE3D;
			lightIndexUavDesc.Texture3D.FirstWSlice		= 0;
			lightIndexUavDesc.Texture3D.MipSlice		= 0;
			lightIndexUavDesc.Texture3D.WSize			= RENDERER_CLUSTER_COUNT_Z;

			D3D11_SHADER_RESOURCE_VIEW_DESC lightIndexSrvDesc;
			ZeroMemory( &lightIndexSrvDesc, sizeof( lightIndexSrvDesc ) );

			lightIndexSrvDesc.Format						= DXGI_FORMAT_R32G32_UINT;
			lightIndexSrvDesc.ViewDimension					= D3D11_SRV_DIMENSION_TEXTURE3D;
			lightIndexSrvDesc.Texture3D.MipLevels			= 1;
			lightIndexSrvDesc.Texture3D.MostDetailedMip		= 0;

			D3D11_UNORDERED_ACCESS_VIEW_DESC lightIdUavDesc;
			ZeroMemory( &lightIdUavDesc, sizeof( lightIdUavDesc ) );

			lightIdUavDesc.Format					= DXGI_FORMAT_R32_UINT;
			lightIdUavDesc.ViewDimension			= D3D11_UAV_DIMENSION_BUFFER;
			lightIdUavDesc.Buffer.FirstElement		= 0;
			lightIdUavDesc.Buffer.Flags				= 0;
			lightIdUavDesc.Buffer.NumElements		= RENDERER_CLUSTER_COUNT_X * RENDERER_CLUSTER_COUNT_Y * RENDERER_CLUSTER_COUNT_Z * RENDERER_CLUSTER_MAX_LIGHTS;

			D3D11_SHADER_RESOURCE_VIEW_DESC lightIdSrvDesc;
			ZeroMemory( &lightIdSrvDesc, sizeof( lightIdSrvDesc ) );

			lightIdSrvDesc.Format						= DXGI_FORMAT_R32_UINT;
			lightIdSrvDesc.ViewDimension				= D3D11_SRV_DIMENSION_BUFFER;
			lightIdSrvDesc.Buffer.ElementOffset			= 0;
			lightIdSrvDesc.Buffer.ElementWidth			= sizeof( UINT );
			lightIdSrvDesc.Buffer.FirstElement			= 0;
			lightIdSrvDesc.Buffer.NumElements			= RENDERER_CLUSTER_COUNT_X * RENDERER_CLUSTER_COUNT_Y * RENDERER_CLUSTER_COUNT_Z * RENDERER_CLUSTER_MAX_LIGHTS;
			
			if( FAILED( inDevice->CreateUnorderedAccessView( m_ClusterInfoBuffer.Get(), &uavDesc, m_ClusterInfoUAV.ReleaseAndGetAddressOf() ) ) ||
				FAILED( inDevice->CreateShaderResourceView( m_ClusterInfoBuffer.Get(), &srvDesc, m_ClusterInfoSRV.ReleaseAndGetAddressOf() ) ) ||
				FAILED( inDevice->CreateUnorderedAccessView( m_ClusterLightIndex.Get(), &lightIndexUavDesc, m_ClusterLightUAV.ReleaseAndGetAddressOf() ) ) ||
				FAILED( inDevice->CreateShaderResourceView( m_ClusterLightIndex.Get(), &lightIndexSrvDesc, m_ClusterLightSRV.ReleaseAndGetAddressOf() ) ) ||
				FAILED( inDevice->CreateUnorderedAccessView( m_LightIdentifierList.Get(), &lightIdUavDesc, m_LightIdentifierUAV.ReleaseAndGetAddressOf() ) ) ||
				FAILED( inDevice->CreateShaderResourceView( m_LightIdentifierList.Get(), &lightIdSrvDesc, m_LightIdentifierSRV.ReleaseAndGetAddressOf() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create view cluster, couldnt create views" );
				Shutdown();
				return false;
			}

			return true;
		}


		bool IsValid() const override
		{
			return m_ClusterInfoBuffer && m_ClusterLightIndex && m_LightIdentifierList;
		}


		void Shutdown() override
		{
			// Cluster Info
			m_ClusterInfoBuffer.Reset();
			m_ClusterInfoUAV.Reset();
			m_ClusterInfoSRV.Reset();

			// Cluster Light Index
			m_ClusterLightIndex.Reset();
			m_ClusterLightUAV.Reset();
			m_ClusterLightSRV.Reset();

			// Light Identifier List
			m_LightIdentifierList.Reset();
			m_LightIdentifierUAV.Reset();
			m_LightIdentifierSRV.Reset();
		}

		inline ID3D11UnorderedAccessView* GetClusterInfoUAV() const { return m_ClusterInfoUAV ? m_ClusterInfoUAV.Get() : nullptr; }
		inline ID3D11ShaderResourceView* GetClusterInfoSRV() const { return m_ClusterInfoSRV ? m_ClusterInfoSRV.Get() : nullptr; }
		inline ID3D11UnorderedAccessView* GetClusterLightUAV() const { return m_ClusterLightUAV ? m_ClusterLightUAV.Get() : nullptr; }
		inline ID3D11ShaderResourceView* GetClusterLightSRV() const { return m_ClusterLightSRV ? m_ClusterLightSRV.Get() : nullptr; }
		inline ID3D11UnorderedAccessView* GetLightIdentifierUAV() const { return m_LightIdentifierUAV ? m_LightIdentifierUAV.Get() : nullptr; }
		inline ID3D11ShaderResourceView* GetLightIdentifierSRV() const { return m_LightIdentifierSRV ? m_LightIdentifierSRV.Get() : nullptr; }

		void MarkDirty() override
		{
			m_bDirty = true;
		}

		void MarkClean() override
		{
			m_bDirty = false;
		}

		bool IsDirty() const override
		{
			return m_bDirty;
		}
	};

}