/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/Shaders/DX11FindClustersShader.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DirectX11/Shaders/DX11FindClustersShader.h"
#include "Hyperion/File/FileSystem.h"
#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Renderer/GBuffer.h"
#include "Hyperion/Renderer/DirectX11/DX11Texture.h"
#include "Hyperion/Renderer/DirectX11/DirectX11ViewClusters.h"


namespace Hyperion
{

	DX11FindClustersShader::DX11FindClustersShader()
		: m_bAttached( false ), m_ScreenWidth( 0 ), m_ScreenHeight( 0 )
	{
	}


	DX11FindClustersShader::~DX11FindClustersShader()
	{
		Shutdown();
	}


	bool DX11FindClustersShader::Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext )
	{
		HYPERION_VERIFY( inDevice && inContext, "[DX11] Device and/or context was null!" );

		// Store context pointer
		m_Context = inContext;

		// Load shader data from file, and create the DX11 shader
		auto shaderFile = FileSystem::OpenFile( FilePath( SHADER_PATH_DX11_COMPUTE_FIND_CLUSTERS, PathRoot::Game ), FileMode::Read );
		auto shaderSize = shaderFile ? shaderFile->GetSize() : 0;

		if( shaderSize <= 0 )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load 'find clusters' shader, file wasnt found or was invalid" );
			return false;
		}

		std::vector< byte > shaderData {};
		{
			DataReader reader( shaderFile );
			reader.SeekBegin();

			if( reader.ReadBytes( shaderData, shaderSize ) != DataReader::ReadResult::Success )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to load 'find clusters' shader, the file couldnt be read" );
				return false;
			}
		}

		// Create the shader through DX11
		if( FAILED( inDevice->CreateComputeShader( shaderData.data(), shaderData.size(), NULL, m_Shader.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load 'find clusters' shader, the shader couldnt be created" );
			Shutdown();
			return false;
		}

		// Create the sampler
		D3D11_SAMPLER_DESC samplerDesc {};

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

		if( FAILED( inDevice->CreateSamplerState( &samplerDesc, m_Sampler.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load 'find clusters' shader, the sampler couldnt be created" );
			Shutdown();
			return false;
		}

		// Create the constant buffer
		D3D11_BUFFER_DESC bufferDesc {};

		bufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags			= 0;
		bufferDesc.ByteWidth			= sizeof( ConstantBufferType );
		bufferDesc.StructureByteStride	= sizeof( ConstantBufferType );

		if( FAILED( inDevice->CreateBuffer( &bufferDesc, NULL, m_ConstantBuffer.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load 'find clusters' shader, the constant buffer couldnt be created" );
			Shutdown();
			return false;
		}

		return true;
	}


	void DX11FindClustersShader::Shutdown()
	{
		m_Shader.Reset();
		m_ConstantBuffer.Reset();
		m_Sampler.Reset();
		m_Context.Reset();
	}


	bool DX11FindClustersShader::IsValid() const
	{
		return m_Shader && m_ConstantBuffer && m_Sampler;
	}


	bool DX11FindClustersShader::UploadStaticParameters( Renderer& inRenderer, uint32 inFlags )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		// Get the values we need...
		auto screenRes = inRenderer.GetResolutionUnsafe();

		// Map the resource
		D3D11_MAPPED_SUBRESOURCE resource {};

		if( FAILED( m_Context->Map( m_ConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to upload static parameters to 'find clusters' shader, mapping failed" );
			return false;
		}

		auto* bufferPtr = (ConstantBufferType*) resource.pData;
		
		bufferPtr->DepthSliceA		= (float) RENDERER_CLUSTER_COUNT_Z / logf( SCREEN_FAR / SCREEN_NEAR );
		bufferPtr->DepthSliceB		= bufferPtr->DepthSliceA * logf( SCREEN_NEAR );
		bufferPtr->ScreenWidth		= (float) screenRes.Width;
		bufferPtr->ScreenHeight		= (float) screenRes.Height;
		bufferPtr->ScreenNear		= SCREEN_NEAR;
		bufferPtr->ScreenFar		= SCREEN_FAR;

		m_Context->Unmap( m_ConstantBuffer.Get(), 0 );

		m_ScreenWidth	= screenRes.Width;
		m_ScreenHeight	= screenRes.Height;

		return true;
	}


	bool DX11FindClustersShader::UploadGBuffer( GBuffer& inGBuffer )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		// Get the depth texture
		DX11Texture2D* depthTexture = dynamic_cast< DX11Texture2D* >( inGBuffer.GetNormalDepthTexture().get() );
		if( depthTexture == nullptr )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to upload GBuffer to 'find clusters' shader, GBuffer was invalid" );
			return false;
		}

		// Upload to the shader
		ID3D11ShaderResourceView* views[] = { depthTexture->GetSRV() };
		m_Context->CSSetShaderResources( 0, 1, views );

		return true;
	}


	bool DX11FindClustersShader::UploadViewClusters( RViewClusters& inClusters )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		// Upload view cluster view
		DirectX11ViewClusters& dx11Clusters = dynamic_cast< DirectX11ViewClusters& >( inClusters );
		
		ID3D11UnorderedAccessView* views[] = { dx11Clusters.GetClusterInfoUAV() };
		m_Context->CSSetUnorderedAccessViews( 0, 1, views, NULL );

		return true;
	}


	bool DX11FindClustersShader::Attach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		m_Context->CSSetShader( m_Shader.Get(), NULL, 0 );
		
		ID3D11SamplerState* samplers[] = { m_Sampler.Get() };
		m_Context->CSSetSamplers( 0, 1, samplers );

		ID3D11Buffer* bufferList[] = { m_ConstantBuffer.Get() };
		m_Context->CSSetConstantBuffers( 0, 1, bufferList );

		m_bAttached = true;

		return true;
	}


	void DX11FindClustersShader::Detach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		ID3D11UnorderedAccessView* uavs[] = { NULL };
		ID3D11ShaderResourceView* srvs[] = { NULL };
		ID3D11Buffer* buffers[] = { NULL };
		ID3D11SamplerState* samplers[] = { NULL };

		m_Context->CSSetUnorderedAccessViews( 0, 1, uavs, NULL );
		m_Context->CSSetShaderResources( 0, 1, srvs );
		m_Context->CSSetConstantBuffers( 0, 1, buffers );
		m_Context->CSSetSamplers( 0, 1, samplers );
		m_Context->CSSetShader( NULL, NULL, 0 );

		m_bAttached = false;
	}


	bool DX11FindClustersShader::Dispatch()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		if( !m_bAttached )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to dispatch 'find clsuters' shader, it wasnt attached" );
			return false;
		}
		
		m_Context->Dispatch( m_ScreenWidth, m_ScreenHeight, 1 );
		return true;
	}
}