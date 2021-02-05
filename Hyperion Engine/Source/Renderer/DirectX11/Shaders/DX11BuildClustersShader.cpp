/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/Shaders/DX11BuildClustersShader.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DirectX11/Shaders/DX11BuildClustersShader.h"
#include "Hyperion/File/FileSystem.h"
#include "Hyperion/Renderer/DirectX11/DirectX11ViewClusters.h"
#include "Hyperion/Renderer/Renderer.h"


namespace Hyperion
{

	DX11BuildClustersShader::DX11BuildClustersShader()
	{

	}


	DX11BuildClustersShader::~DX11BuildClustersShader()
	{
		Shutdown();
	}


	bool DX11BuildClustersShader::Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext )
	{
		HYPERION_VERIFY( inDevice && inContext, "[DX11] Device and/or context was null" );

		// Store context pointer
		m_Context = inContext;

		// Load shader data from file, and create the DX11 shader
		auto shaderFile = FileSystem::OpenFile( FilePath( SHADER_PATH_DX11_COMPUTE_BUILD_CLUSTERS, PathRoot::Game ), FileMode::Read );
		auto shaderSize = shaderFile ? shaderFile->GetSize() : 0;

		if( shaderSize <= 0 )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load 'build clusters' shader, file wasnt found or was invalid" );
			return false;
		}

		std::vector< byte > shaderData {};
		{
			DataReader reader( shaderFile );
			reader.SeekBegin();

			if( reader.ReadBytes( shaderData, shaderSize ) != DataReader::ReadResult::Success )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to load 'build clusters' shader, the file couldnt be read" );
				return false;
			}
		}

		// Create the shader through DX11
		if( FAILED( inDevice->CreateComputeShader( shaderData.data(), shaderData.size(), NULL, m_Shader.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load 'build clusters' shader, the shader couldnt be created" );
			Shutdown();
			return false;
		}

		D3D11_BUFFER_DESC bufferDesc {};

		bufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags			= 0;
		bufferDesc.StructureByteStride	= sizeof( ConstantBufferType );
		bufferDesc.ByteWidth			= sizeof( ConstantBufferType );

		if( FAILED( inDevice->CreateBuffer( &bufferDesc, NULL, m_ConstantBuffer.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load 'build clusters' shader, the constant buffer couldnt be created" );
			Shutdown();
			return false;
		}

		return true;
	}


	void DX11BuildClustersShader::Shutdown()
	{
		m_Shader.Reset();
		m_Context.Reset();
		m_ConstantBuffer.Reset();
	}


	bool DX11BuildClustersShader::IsValid() const
	{
		return m_Shader && m_ConstantBuffer;
	}


	bool DX11BuildClustersShader::UploadStaticParameters( Renderer& inRenderer, uint32 inFlags )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		// Get the parameters we need..
		auto screenRes = inRenderer.GetResolutionUnsafe();

		DirectX::XMMATRIX projMatrix( inRenderer.GetProjectionMatrix().GetData() );
		projMatrix = DirectX::XMMatrixInverse( nullptr, projMatrix );

		D3D11_MAPPED_SUBRESOURCE resource {};
		if( FAILED( m_Context->Map( m_ConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to upload static parameters to 'build clusters' shader, mapping failed" );
			return false;
		}

		auto bufferPtr = (ConstantBufferType*) resource.pData;

		bufferPtr->ScreenWidth			= (float) screenRes.Width;
		bufferPtr->ScreenHeight			= (float) screenRes.Height;
		bufferPtr->ScreenNear			= SCREEN_NEAR;
		bufferPtr->ScreenFar			= SCREEN_FAR;
		bufferPtr->ShaderMode			= inFlags;
		bufferPtr->InvProjectionMatrix	= DirectX::XMMatrixTranspose( projMatrix );
		
		m_Context->Unmap( m_ConstantBuffer.Get(), 0 );
		
		ID3D11Buffer* bufferList[] = { m_ConstantBuffer.Get() };
		m_Context->CSSetConstantBuffers( 0, 1, bufferList );

		return true;
	}


	bool DX11BuildClustersShader::UploadViewClusters( RViewClusters& inClusters )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context is null" );
		
		// Get unordered access view for the cluster buffer
		DirectX11ViewClusters& dx11Clusters = dynamic_cast< DirectX11ViewClusters& >( inClusters );
		auto* clusterView = dx11Clusters.GetClusterInfoUAV();

		if( clusterView == nullptr )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to upload view clusters to 'build clusters' compute shader, UAV was null" );
			return false;
		}

		ID3D11UnorderedAccessView* uavList[] = { clusterView };
		m_Context->CSSetUnorderedAccessViews( 0, 1, uavList, NULL );

		return true;
	}


	bool DX11BuildClustersShader::Attach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context is null" );
		m_Context->CSSetShader( m_Shader.Get(), NULL, 0 );

		m_bAttached = true;
		return true;
	}


	void DX11BuildClustersShader::Detach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context is null" );

		ID3D11UnorderedAccessView* uavList[] = { NULL };

		m_Context->CSSetUnorderedAccessViews( 0, 1, uavList, NULL );
		m_Context->CSSetShader( NULL, NULL, 0 );

		m_bAttached = false;
	}


	bool DX11BuildClustersShader::Dispatch()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		// Ensure the shader is attached
		if( !m_bAttached )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to dispatch 'build clusters' shader, it wasnt attached" );
			return false;
		}

		m_Context->Dispatch( 24, 1, 1 );
		return true;
	}


}