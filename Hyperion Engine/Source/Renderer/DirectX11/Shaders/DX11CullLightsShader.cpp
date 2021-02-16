/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/Shaders/DX11CullLightsShader.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DirectX11/Shaders/DX11CullLightsShader.h"
#include "Hyperion/File/FileSystem.h"
#include "Hyperion/Renderer/DirectX11/DirectX11ViewClusters.h"
#include "Hyperion/Renderer/DirectX11/DirectX11LightBuffer.h"
#include "Hyperion/Renderer/Renderer.h"


namespace Hyperion
{

	DX11CullLightsShader::DX11CullLightsShader()
		: m_bAttached( false )
	{

	}


	DX11CullLightsShader::~DX11CullLightsShader()
	{
		Shutdown();
	}


	bool DX11CullLightsShader::Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext )
	{
		HYPERION_VERIFY( inDevice && inContext, "[DX11] Device and/or context was null" );

		// Store context pointer
		m_Context = inContext;

		// Load shader data from file, and create the DX11 shader
		auto shaderFile = FileSystem::OpenFile( FilePath( SHADER_PATH_DX11_COMPUTE_CULL_LIGHTS, PathRoot::Game ), FileMode::Read );
		auto shaderSize = shaderFile ? shaderFile->GetSize() : 0;

		if( shaderSize <= 0 )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load 'cull lights' shader, file wasnt found or was invalid" );
			return false;
		}

		std::vector< byte > shaderData {};
		{
			DataReader reader( shaderFile );
			reader.SeekBegin();

			if( reader.ReadBytes( shaderData, shaderSize ) != DataReader::ReadResult::Success )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to load 'cull lights' shader, the file couldnt be read" );
				return false;
			}
		}

		// Create the shader through DX11
		if( FAILED( inDevice->CreateComputeShader( shaderData.data(), shaderData.size(), NULL, m_Shader.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load 'cull lights' shader, the shader couldnt be created" );
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
			Console::WriteLine( "[ERROR] DX11: Failed to load 'cull lights' shader, the constant buffer couldnt be created" );
			Shutdown();
			return false;
		}

		return true;
	}


	void DX11CullLightsShader::Shutdown()
	{
		m_Shader.Reset();
		m_ConstantBuffer.Reset();
		m_Context.Reset();
	}


	bool DX11CullLightsShader::IsValid() const
	{
		return m_Shader && m_ConstantBuffer;
	}


	bool DX11CullLightsShader::UploadLightBuffer( RLightBuffer& inLights )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		DirectX11LightBuffer& dx11Buffer		= dynamic_cast< DirectX11LightBuffer& >( inLights );
		ID3D11ShaderResourceView* lightView		= dx11Buffer.GetView();

		if( lightView == nullptr )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to upload light buffer to 'cull lights' shader, light buffer was invalid" );
			return false;
		}
		
		ID3D11ShaderResourceView* resources[] = { lightView };
		m_Context->CSSetShaderResources( 1, 1, resources );

		return true;
	}


	bool DX11CullLightsShader::UploadStaticParameters( Renderer& inRenderer, uint32 inFlags )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		// Get the values we need
		DirectX::XMMATRIX viewMatrix( inRenderer.GetViewMatrix().GetData() );
		uint32 lightCount = inRenderer.GetLightBuffer()->GetLightCount();

		D3D11_MAPPED_SUBRESOURCE resource {};

		if( FAILED( m_Context->Map( m_ConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to upload static parameters to 'cull lights' shader, mapping failed" );
			return false;
		}

		auto* bufferPtr = (ConstantBufferType*) resource.pData;

		bufferPtr->LightCount	= lightCount;
		bufferPtr->ViewMatrix	= DirectX::XMMatrixTranspose( viewMatrix );

		m_Context->Unmap( m_ConstantBuffer.Get(), 0 );

		return true;
	}


	bool DX11CullLightsShader::UploadViewClusters( RViewClusters& inClusters )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		// Get the values we need
		DirectX11ViewClusters& clusters = dynamic_cast< DirectX11ViewClusters& >( inClusters );

		auto* clusterInfoSRV	= clusters.GetClusterInfoSRV();
		auto* lightListUAV		= clusters.GetLightIdentifierUAV();
		auto* clusterLightUAV	= clusters.GetClusterLightUAV();

		if( clusterInfoSRV == nullptr || lightListUAV == nullptr || clusterLightUAV == nullptr )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to upload view clusters to 'cull lights' shader, clusters were null" );
			return false;
		}
		
		ID3D11ShaderResourceView* resources[]	= { clusterInfoSRV };
		ID3D11UnorderedAccessView* uavs[]		= { clusterLightUAV, lightListUAV };

		m_Context->CSSetShaderResources( 0, 1, resources );
		m_Context->CSSetUnorderedAccessViews( 0, 2, uavs, NULL );

		return true;
	}


	bool DX11CullLightsShader::Attach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		m_Context->CSSetShader( m_Shader.Get(), NULL, 0 );

		ID3D11Buffer* bufferList[] = { m_ConstantBuffer.Get() };
		m_Context->CSSetConstantBuffers( 0, 1, bufferList );

		m_bAttached = true;

		return true;
	}


	void DX11CullLightsShader::Detach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		ID3D11ShaderResourceView* resources[]	= { NULL, NULL };
		ID3D11UnorderedAccessView* uavs[]		= { NULL, NULL };
		ID3D11Buffer* buffers[]					= { NULL };

		m_Context->CSSetUnorderedAccessViews( 0, 2, uavs, NULL );
		m_Context->CSSetShaderResources( 0, 2, resources );
		m_Context->CSSetConstantBuffers( 0, 1, buffers );
		m_Context->CSSetShader( NULL, NULL, 0 );

		m_bAttached = false;
	}


	bool DX11CullLightsShader::Dispatch()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		if( !m_bAttached )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to dispatch 'cull lights' shader, it isnt attached" );
			return false;
		}

		m_Context->Dispatch( 15, 10, 24 );
		return true;
	}

}