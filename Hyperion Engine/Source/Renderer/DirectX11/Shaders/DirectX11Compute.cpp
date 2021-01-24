/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/Shaders/DirectX11Compute.cpp
	© 2021, Zachary Berry
==================================================================================================*/


#include "Hyperion/Renderer/DirectX11/Shaders/DirectX11Compute.h"
#include "Hyperion/File/FileSystem.h"


namespace Hyperion
{

	DirectX11BuildClusterShader::DirectX11BuildClusterShader( const String& inShaderPath )
		: m_ShaderPath( inShaderPath ), m_ClusterCountX( RENDERER_CLUSTER_COUNT_X ), m_ClusterCountY( RENDERER_CLUSTER_COUNT_Y ), m_ClusterCountZ( RENDERER_CLUSTER_COUNT_Z ),
		m_ClusterSizeX( 0.f ), m_ClusterSizeY( 0.f )
	{

	}

	DirectX11BuildClusterShader::~DirectX11BuildClusterShader()
	{
		Shutdown();
	}


	bool DirectX11BuildClusterShader::Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext )
	{
		HYPERION_VERIFY( inDevice != nullptr && inContext != nullptr, "[DX11] Attempt to create shader with invalid device/context" );
		m_Context = inContext;

		// We need to read the shader file into memory
		auto f = FileSystem::OpenFile( FilePath( m_ShaderPath, PathRoot::Game ), FileMode::Read );

		if( !f || !f->IsValid() )
		{
			Console::WriteLine( "[ERROR] DX11: Failed top create 'build cluster' shader, the shader path was invalid \"", m_ShaderPath, "\"" );
			return false;
		}

		std::vector< byte > shaderData;

		{
			DataReader reader( f );
			reader.SeekBegin();

			if( reader.ReadBytes( shaderData, reader.Size() ) != DataReader::ReadResult::Success )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create 'build cluster' shader, the shader file couldnt be read \"", m_ShaderPath, "\"" );
				return false;
			}
		}

		// Now, we need to create the shader in DX11
		if( FAILED( inDevice->CreateComputeShader( shaderData.data(), shaderData.size(), NULL, m_Shader.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 'build cluster' shader, DX11 create shader call failed!" );
			return false;
		}

		// And finally, create resources we need for this shader
		D3D11_BUFFER_DESC screenBufferDesc;
		ZeroMemory( &screenBufferDesc, sizeof( screenBufferDesc ) );

		screenBufferDesc.Usage					= D3D11_USAGE_DYNAMIC;
		screenBufferDesc.ByteWidth				= sizeof( ClusterShaderScreenBuffer );
		screenBufferDesc.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
		screenBufferDesc.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;
		screenBufferDesc.MiscFlags				= 0;
		screenBufferDesc.StructureByteStride	= 0;

		if( FAILED( inDevice->CreateBuffer( &screenBufferDesc, NULL, m_ScreenInfoBuffer.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 'build cluster' shader, couldnt create screen info buffer" );
			return false;
		}
		
		return true;
	}


	void DirectX11BuildClusterShader::Shutdown()
	{
		m_ScreenInfoBuffer.Reset();
		m_Shader.Reset();
	}


	bool DirectX11BuildClusterShader::UploadViewInfo( const Matrix& inProjectionMatrix, const ScreenResolution& inResolution, float inScreenNear, float inScreenFar )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		// Calculate projection inverse
		DirectX::XMMATRIX projectionMatrix( inProjectionMatrix.GetData() );
		projectionMatrix = DirectX::XMMatrixInverse( nullptr, projectionMatrix );
		projectionMatrix = DirectX::XMMatrixTranspose( projectionMatrix );

		// Calculate cluster size
		m_ClusterSizeX = ceilf( (float) inResolution.Width / (float) m_ClusterCountX );
		m_ClusterSizeY = ceilf( (float) inResolution.Height / (float) m_ClusterCountY );

		// Map constant buffer to upload parameters
		D3D11_MAPPED_SUBRESOURCE resource{};

		if( FAILED( m_Context->Map( m_ScreenInfoBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to map ScreenInfoBuffer in 'Build Cluster' shader!" );
			return false;
		}

		auto* screenInfo = (ClusterShaderScreenBuffer*) resource.pData;

		screenInfo->InvProjection	= projectionMatrix;
		screenInfo->ClusterCount	= DirectX::XMUINT3( m_ClusterCountX, m_ClusterCountY, m_ClusterCountZ );
		screenInfo->ClusterSize		= DirectX::XMUINT2( (uint32)m_ClusterSizeX, (uint32)m_ClusterSizeY );
		screenInfo->ScreenSize		= DirectX::XMUINT2( inResolution.Width, inResolution.Height );
		screenInfo->NearZ			= inScreenNear;
		screenInfo->FarZ			= inScreenFar;

		m_Context->Unmap( m_ScreenInfoBuffer.Get(), 0 );

		ID3D11Buffer* bufferList[] = { m_ScreenInfoBuffer.Get() };
		m_Context->CSSetConstantBuffers( 0, 1, bufferList );

		return true;
	}


	void DirectX11BuildClusterShader::Detach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		ID3D11UnorderedAccessView* uavList[] = { NULL };
		m_Context->CSSetUnorderedAccessViews( 0, 1, uavList, NULL );
		m_Context->CSSetShader( NULL, NULL, 0 );
	}


	void DirectX11BuildClusterShader::Attach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null!" );
		m_Context->CSSetShader( m_Shader.Get(), NULL, 0 );
	}


	bool DirectX11BuildClusterShader::Dispatch( const std::shared_ptr< RViewClusters >& outClusters )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null!" );

		// Cast clusters to api type
		DirectX11ViewClusters* dx11Clusters = dynamic_cast< DirectX11ViewClusters* >( outClusters.get() );
		HYPERION_VERIFY( dx11Clusters != nullptr, "[DX11] API type mismatch?" );

		ID3D11UnorderedAccessView* uav = dx11Clusters->GetClusterInfoUAV();
		if( !uav )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to dispatch 'Build Clusters' shader, the output structure couldnt be bound!" );
			return false;
		}

		// Set the UAV
		ID3D11UnorderedAccessView* uavList[] = { uav };
		m_Context->CSSetUnorderedAccessViews( 0, 1, uavList, NULL );

		// And cache cluster size (in pixels) in the cluster object itself, so further into the frame, we have access to these values
		outClusters->ClusterSizeX = m_ClusterSizeX;
		outClusters->ClusterSizeY = m_ClusterSizeY;

		// And finally, dispatch the shader
		m_Context->Dispatch( RENDERER_CLUSTER_COUNT_Z, 0, 0 );
		return true;
	}


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	DirectX11CompressClustersShader::DirectX11CompressClustersShader( const String& inShaderPath )
		: m_ShaderPath( inShaderPath ), m_Context( NULL )
	{
	}


	DirectX11CompressClustersShader::~DirectX11CompressClustersShader()
	{
		Shutdown();
	}


	bool DirectX11CompressClustersShader::Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext )
	{
		HYPERION_VERIFY( inDevice && inContext, "[DX11] Device and/or context were null!" );
		m_Context = inContext;

		// We need to load the shader data from disk, and create it using DX11
		auto f = FileSystem::OpenFile( FilePath( m_ShaderPath, PathRoot::Game ), FileMode::Read );

		if( !f || !f->IsValid() )
		{
			Console::WriteLine( "[ERROR] DX11: Failed top create 'compress cluster' shader, the shader path was invalid \"", m_ShaderPath, "\"" );
			return false;
		}

		std::vector< byte > shaderData;

		{
			DataReader reader( f );
			reader.SeekBegin();

			if( reader.ReadBytes( shaderData, reader.Size() ) != DataReader::ReadResult::Success )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to create 'compress cluster' shader, the shader file couldnt be read \"", m_ShaderPath, "\"" );
				return false;
			}
		}

		// Now, we need to create the shader in DX11
		if( FAILED( inDevice->CreateComputeShader( shaderData.data(), shaderData.size(), NULL, m_Shader.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to create 'compress cluster' shader, DX11 create shader call failed!" );
			return false;
		}

		return true;
	}


	void DirectX11CompressClustersShader::Shutdown()
	{
		m_Shader.Reset();
	}


	void DirectX11CompressClustersShader::Attach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Context was null!" );
		m_Context->CSSetShader( m_Shader.Get(), NULL, 0 );
	}


	void DirectX11CompressClustersShader::Detach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null!" );

		ID3D11ShaderResourceView* resourceList[] = { NULL };
		ID3D11UnorderedAccessView* uavList[] = { NULL };

		m_Context->CSSetShaderResources( 0, 1, resourceList );
		m_Context->CSSetUnorderedAccessViews( 0, 1, uavList, NULL );
		m_Context->CSSetShader( NULL, NULL, 0 );
	}


	bool DirectX11CompressClustersShader::Dispatch( const std::shared_ptr< RViewClusters >& inClusters )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null!" );

		DirectX11ViewClusters* clusters = dynamic_cast< DirectX11ViewClusters* >( inClusters.get() );
		if( !clusters )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to dispatch 'Compress Clusters' shader, the output cluster structure was null/invalid" );
			return false;
		}

		ID3D11ShaderResourceView* resourceList[] = { clusters->GetClusterInfoSRV() };
		m_Context->CSSetShaderResources( 0, 1, resourceList );

		ID3D11UnorderedAccessView* uavList[] = { clusters->GetActiveClusterUAV() };
		m_Context->CSSetUnorderedAccessViews( 0, 1, uavList, NULL );

		m_Context->Dispatch( RENDERER_CLUSTER_COUNT_Z, 0, 0 );
		return true;
	}


}