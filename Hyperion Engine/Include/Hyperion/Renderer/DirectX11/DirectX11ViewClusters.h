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
		DirectX::XMFLOAT3 MinAABB;
		float _pad_1;
		DirectX::XMFLOAT3 MaxAABB;
		float _pad_2;
		uint32 bActive;
		DirectX::XMFLOAT3 _pad_3;
	};


	class DirectX11ViewClusters : public RViewClusters
	{

	private:

		Microsoft::WRL::ComPtr< ID3D11Buffer > m_ActiveClusterBuffer;
		Microsoft::WRL::ComPtr< ID3D11UnorderedAccessView > m_ActiveClusterUAV;
		Microsoft::WRL::ComPtr< ID3D11Buffer > m_ClusterInfoBuffer;
		Microsoft::WRL::ComPtr< ID3D11UnorderedAccessView > m_ClusterInfoUAV;
		Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > m_ClusterInfoSRV;
		Microsoft::WRL::ComPtr< ID3D11Buffer > m_StagingBuffer;
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

			D3D11_BUFFER_DESC activeClusterDesc;
			ZeroMemory( &activeClusterDesc, sizeof( activeClusterDesc ) );

			activeClusterDesc.Usage					= D3D11_USAGE_DEFAULT;
			activeClusterDesc.ByteWidth				= sizeof( UINT ) * RENDERER_CLUSTER_COUNT_X * RENDERER_CLUSTER_COUNT_Y * RENDERER_CLUSTER_COUNT_Z;
			activeClusterDesc.BindFlags				= D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
			activeClusterDesc.CPUAccessFlags		= 0;
			activeClusterDesc.MiscFlags				= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			activeClusterDesc.StructureByteStride	= sizeof( UINT );


			// TODO: Set initial values to zero?
			if( FAILED( inDevice->CreateBuffer( &clusterInfoDesc, NULL, m_ClusterInfoBuffer.ReleaseAndGetAddressOf() ) ) ||
				FAILED( inDevice->CreateBuffer( &activeClusterDesc, NULL, m_ActiveClusterBuffer.ReleaseAndGetAddressOf() ) ) )
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

			D3D11_UNORDERED_ACCESS_VIEW_DESC activeUavDesc;
			ZeroMemory( &activeUavDesc, sizeof( activeUavDesc ) );

			activeUavDesc.Buffer.FirstElement	= 0;
			activeUavDesc.Buffer.Flags			= D3D11_BUFFER_UAV_FLAG_APPEND;
			activeUavDesc.Buffer.NumElements	= RENDERER_CLUSTER_COUNT_X * RENDERER_CLUSTER_COUNT_Y * RENDERER_CLUSTER_COUNT_Z;
			activeUavDesc.Format				= DXGI_FORMAT_UNKNOWN;
			activeUavDesc.ViewDimension			= D3D11_UAV_DIMENSION_BUFFER;

			if( FAILED( inDevice->CreateUnorderedAccessView( m_ClusterInfoBuffer.Get(), &uavDesc, m_ClusterInfoUAV.ReleaseAndGetAddressOf() ) ) ||
				FAILED( inDevice->CreateShaderResourceView( m_ClusterInfoBuffer.Get(), &srvDesc, m_ClusterInfoSRV.ReleaseAndGetAddressOf() ) )  ||
				FAILED( inDevice->CreateUnorderedAccessView( m_ActiveClusterBuffer.Get(), &activeUavDesc, m_ActiveClusterUAV.ReleaseAndGetAddressOf() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create view cluster, couldnt create views" );
				Shutdown();
				return false;
			}

			return true;
		}


		bool IsValid() const override
		{
			return m_ClusterInfoBuffer && m_ClusterInfoUAV && m_ClusterInfoSRV && m_ActiveClusterBuffer && m_ActiveClusterUAV;
		}


		void Shutdown() override
		{
			m_ActiveClusterUAV.Reset();
			m_ActiveClusterBuffer.Reset();
			m_StagingBuffer.Reset();
			m_ClusterInfoBuffer.Reset();
			m_ClusterInfoUAV.Reset();
			m_ClusterInfoSRV.Reset();
		}

		inline ID3D11Buffer* GetClusterInfoBuffer() const { return m_ClusterInfoBuffer ? m_ClusterInfoBuffer.Get() : nullptr; }
		inline ID3D11UnorderedAccessView* GetClusterInfoUAV() const { return m_ClusterInfoUAV ? m_ClusterInfoUAV.Get() : nullptr; }
		inline ID3D11ShaderResourceView* GetClusterInfoSRV() const { return m_ClusterInfoSRV ? m_ClusterInfoSRV.Get() : nullptr; }
		inline ID3D11UnorderedAccessView* GetActiveClusterUAV() const { return m_ActiveClusterUAV ? m_ActiveClusterUAV.Get() : nullptr; }

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