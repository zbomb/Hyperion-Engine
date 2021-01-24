/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/Shaders/DirectX11GBufferShader.cpp
	© 2021, Zachary Berry
==================================================================================================*/


#include "Hyperion/Renderer/DirectX11/Shaders/DirectX11GBufferShader.h"
#include "Hyperion/File/FileSystem.h"
#include "Hyperion/Renderer/Resource/Material.h"
#include "Hyperion/Renderer/DirectX11/DirectX11Texture.h"
#include "Hyperion/Library/Geometry.h"


namespace Hyperion
{

	DirectX11GBufferShader::DirectX11GBufferShader( const String& inPixelShader, const String& inVertexShader )
		: m_PixelShaderPath( inPixelShader ), m_VertexShaderPath( inVertexShader )
	{

	}


	DirectX11GBufferShader::~DirectX11GBufferShader()
	{
		Shutdown();
	}


	bool DirectX11GBufferShader::Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext )
	{
		HYPERION_VERIFY( inDevice != nullptr && inContext != nullptr, "[DX11] Failed to create shader, device was nullptr" );
		m_Context = inContext;

		// First, lets get the data for both shaders
		if( m_PixelShaderPath.IsWhitespaceOrEmpty() || m_VertexShaderPath.IsWhitespaceOrEmpty() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create GBuffer shader, path(s) were invalid" );
			return false;
		}

		auto pixelFile		= FileSystem::OpenFile( FilePath( m_PixelShaderPath, PathRoot::Game ), FileMode::Read );
		auto vertexFile		= FileSystem::OpenFile( FilePath( m_VertexShaderPath, PathRoot::Game ), FileMode::Read );

		if( !pixelFile || !vertexFile || !pixelFile->IsValid() || !vertexFile->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create GBuffer shader, the shaders werent found on disk" );
			return false;
		}

		std::vector< byte > pixelShaderData;
		std::vector< byte > vertexShaderData;

		{
			DataReader reader( pixelFile );
			reader.SeekBegin();

			if( reader.ReadBytes( pixelShaderData, reader.Size() ) != DataReader::ReadResult::Success || pixelShaderData.size() == 0 )
			{
				Console::WriteLine( "[Warning] DX11: Failed to create GBuffer shader, the pixel shader couldnt be read" );
				return false;
			}
		}
		{
			DataReader reader( vertexFile );

			if( reader.ReadBytes( vertexShaderData, reader.Size() ) != DataReader::ReadResult::Success || vertexShaderData.size() == 0 )
			{
				Console::WriteLine( "[Warning] DX11: Failed to create GBuffer shader, the vertex shader couldnt be read" );
				return false;
			}
		}

		// Now, lets use DX11 to create the shaders
		if( FAILED( inDevice->CreateVertexShader( vertexShaderData.data(), vertexShaderData.size(), NULL, m_VertexShader.GetAddressOf() ) ) || m_VertexShader.Get() == NULL )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create GBuffer shader, the vertex shader couldnt be created" );
			return false;
		}

		if( FAILED( inDevice->CreatePixelShader( pixelShaderData.data(), pixelShaderData.size(), NULL, m_PixelShader.GetAddressOf() ) ) || m_PixelShader.Get() == NULL )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create GBuffer shader, the pixel shader couldnt be created" );
			Shutdown();

			return false;
		}

		// Next, create the input layouts, so DX11 knows how to pass data into the vertex shader
		D3D11_INPUT_ELEMENT_DESC inputDesc[ 3 ];

		inputDesc[ 0 ].SemanticName				= "POSITION";
		inputDesc[ 0 ].SemanticIndex			= 0;
		inputDesc[ 0 ].Format					= DXGI_FORMAT_R32G32B32_FLOAT;
		inputDesc[ 0 ].InputSlot				= 0;
		inputDesc[ 0 ].AlignedByteOffset		= 0;
		inputDesc[ 0 ].InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;
		inputDesc[ 0 ].InstanceDataStepRate		= 0;

		inputDesc[ 1 ].SemanticName				= "NORMAL";
		inputDesc[ 1 ].SemanticIndex			= 0;
		inputDesc[ 1 ].Format					= DXGI_FORMAT_R32G32B32_FLOAT;
		inputDesc[ 1 ].InputSlot				= 0;
		inputDesc[ 1 ].AlignedByteOffset		= D3D11_APPEND_ALIGNED_ELEMENT;
		inputDesc[ 1 ].InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;
		inputDesc[ 1 ].InstanceDataStepRate		= 0;

		inputDesc[ 2 ].SemanticName				= "TEXCOORD";
		inputDesc[ 2 ].SemanticIndex			= 0;
		inputDesc[ 2 ].Format					= DXGI_FORMAT_R32G32_FLOAT;
		inputDesc[ 2 ].InputSlot				= 0;
		inputDesc[ 2 ].AlignedByteOffset		= D3D11_APPEND_ALIGNED_ELEMENT;
		inputDesc[ 2 ].InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;
		inputDesc[ 2 ].InstanceDataStepRate		= 0;

		if( FAILED( inDevice->CreateInputLayout( inputDesc, 3, vertexShaderData.data(), vertexShaderData.size(), m_InputLayout.GetAddressOf() ) ) || m_InputLayout.Get() == NULL )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create GBuffer shader, the input layout couldnt be created" );
			Shutdown();

			return false;
		}

		// Now, we need to setup samplers and buffers
		D3D11_BUFFER_DESC matrixBufferDesc;
		ZeroMemory( &matrixBufferDesc, sizeof( matrixBufferDesc ) );

		matrixBufferDesc.Usage					= D3D11_USAGE_DYNAMIC;
		matrixBufferDesc.ByteWidth				= sizeof( MatrixBuffer );
		matrixBufferDesc.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
		matrixBufferDesc.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;
		matrixBufferDesc.MiscFlags				= 0;
		matrixBufferDesc.StructureByteStride	= 0;

		if( FAILED( inDevice->CreateBuffer( &matrixBufferDesc, NULL, m_MatrixBuffer.GetAddressOf() ) ) || m_MatrixBuffer.Get() == NULL )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create GBuffer shader, the matrix buffer coudnt be created" );
			Shutdown();

			return false;
		}

		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory( &samplerDesc, sizeof( samplerDesc ) );

		// Create texture sampler description
		samplerDesc.Filter				= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU			= D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV			= D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW			= D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias			= 0.0f;
		samplerDesc.MaxAnisotropy		= 1;
		samplerDesc.ComparisonFunc		= D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[ 0 ]	= 0;
		samplerDesc.BorderColor[ 1 ]	= 0;
		samplerDesc.BorderColor[ 2 ]	= 0;
		samplerDesc.BorderColor[ 3 ]	= 0;
		samplerDesc.MinLOD				= 0;
		samplerDesc.MaxLOD				= D3D11_FLOAT32_MAX;

		if( FAILED( inDevice->CreateSamplerState( &samplerDesc, m_SamplerState.GetAddressOf() ) ) || m_SamplerState.Get() == NULL )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create GBuffer shader, the sampler couldnt be created" );
			Shutdown();

			return false;
		}

		Console::WriteLine( "DX11: GBuffer shader initialized successfully!" );
		return true;
	}


	void DirectX11GBufferShader::Shutdown()
	{
		m_SamplerState.Reset();
		m_MatrixBuffer.Reset();
		m_InputLayout.Reset();
		m_PixelShader.Reset();
		m_VertexShader.Reset();
	}


	bool DirectX11GBufferShader::IsValid() const
	{
		return m_PixelShader && m_VertexShader;
	}


	bool DirectX11GBufferShader::UploadMatrixData( const Matrix& worldMatrix, const Matrix& viewMatrix, const Matrix& projectionMatrix )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Cached context was null!" );

		// Setup the matricies to be uploaded to the graphics card
		DirectX::XMMATRIX world( worldMatrix.GetData() );
		DirectX::XMMATRIX view( viewMatrix.GetData() );
		DirectX::XMMATRIX projection( projectionMatrix.GetData() );

		// Transpose the matricies before writing to GPU
		view = DirectX::XMMatrixTranspose( view );
		world = DirectX::XMMatrixTranspose( world  );
		projection = DirectX::XMMatrixTranspose( projection );

		D3D11_MAPPED_SUBRESOURCE mappedResource{};
		auto res = m_Context->Map( m_MatrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );

		if( FAILED( res ) )
		{
			Console::WriteLine( "[Warning] DX11: Failed to map matrix buffer!" );
			return false;
		}

		MatrixBuffer* bufferPtr		= (MatrixBuffer*) mappedResource.pData;
		bufferPtr->World			= world;
		bufferPtr->View				= view;
		bufferPtr->Projection		= projection;

		m_Context->Unmap( m_MatrixBuffer.Get(), 0 );

		// Set the buffer in the shader
		ID3D11Buffer* bufferList[] = { m_MatrixBuffer.Get() };
		m_Context->VSSetConstantBuffers( 0, 1, bufferList );

		return true;
	}


	bool DirectX11GBufferShader::UploadMaterial( const std::shared_ptr<RMaterial>& inMaterial )
	{
		HYPERION_VERIFY( m_Context != nullptr, "[DX11] Cached context was null!" );

		// Validate the material being passed
		if( !inMaterial )
		{
			Console::WriteLine( "[Warning] DX11: Failed to upload material to GBuffer shader! Material was null" );
			return false;
		}

		// For now, were only going to use the 'base_map" field, to determine which texture to apply
		// Later on, we will apply more stuff
		auto texBase = std::dynamic_pointer_cast<DirectX11Texture2D>( inMaterial->GetTexture( "base_map" ) );
		
		if( !texBase || !texBase->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to upload material to GBuffer shader! Base map texture was invalid" );
			return false;
		}

		ID3D11ShaderResourceView* view[] = { texBase->GetView() };
		m_Context->PSSetShaderResources( 0, 1, view );

		ID3D11SamplerState* samplerList[] = { m_SamplerState.Get() };
		m_Context->PSSetSamplers( 0, 1, samplerList );

		return true;
	}


	bool DirectX11GBufferShader::UploadClusterData( const std::shared_ptr< RViewClusters >& inClusters )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Context was null!" );

		if( !inClusters )
		{
			Console::WriteLine( "[Warning] DX11: Failed to upload cluster info to gbuffer shader! Cluster structure was null" );
			return false;
		}

		DirectX11ViewClusters* dx11Clusters		= dynamic_cast<DirectX11ViewClusters*>( inClusters.get() );

		D3D11_MAPPED_SUBRESOURCE resource{};

		if( FAILED( m_Context->Map( m_ClusterInfoBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) ) )
		{
			Console::WriteLine( "[Warning] DX11: Failed to upload cluster info to GBuffer shader! Failed to map buffer" );
			return false;
		}

		auto* bufferPtr = (ClusterInfoBuffer*) resource.pData;

		bufferPtr->clusterCountX	= RENDERER_CLUSTER_COUNT_X;
		bufferPtr->clusterCountY	= RENDERER_CLUSTER_COUNT_Y;
		bufferPtr->clusterSizeX		= inClusters->ClusterSizeX;
		bufferPtr->clusterSizeY		= inClusters->ClusterSizeY;
		bufferPtr->depthCalcTermA	= (float)RENDERER_CLUSTER_COUNT_Z / logf( SCREEN_NEAR / SCREEN_FAR );
		bufferPtr->depthCalcTermB	= bufferPtr->depthCalcTermA * logf( SCREEN_NEAR );
		
		m_Context->Unmap( m_ClusterInfoBuffer.Get(), 0 );

		ID3D11Buffer* bufferList[] = { m_ClusterInfoBuffer.Get() };
		m_Context->PSSetConstantBuffers( 0, 1, bufferList );

		// The actual cluster list is uploaded with the render targets, when the GBuffer is set as the render target (inside DirectX11Graphics)
		return true;
	}

}