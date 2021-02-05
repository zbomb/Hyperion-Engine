/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/Shaders/DirectX11LightBuffer.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Renderer/LightBuffer.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


namespace Hyperion
{

	class DirectX11LightBuffer : public RLightBuffer
	{

	private:

		uint32 m_MaxLights;
		uint32 m_CurrentLights;
		Microsoft::WRL::ComPtr< ID3D11Buffer > m_LightBuffer;
		Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > m_LightBufferView;
		ID3D11DeviceContext* m_Context;
		

	public:

		DirectX11LightBuffer() = delete;

		DirectX11LightBuffer( uint32 maxLights )
			: m_MaxLights( maxLights ), m_CurrentLights( 0 ), m_Context( nullptr )
		{
			HYPERION_VERIFY( maxLights > 0, "[DX11] Max dynamic lights value is invalid" );
		}


		~DirectX11LightBuffer()
		{
			Shutdown();
		}


		void Shutdown() final
		{
			m_LightBufferView.Reset();
			m_LightBuffer.Reset();

			m_Context = nullptr;
		}


		inline uint32 GetLightCount() const final { return m_CurrentLights; }


		bool IsValid() const final
		{
			return m_LightBuffer && m_LightBufferView;
		}

		inline ID3D11Buffer* GetBuffer() { return m_LightBuffer.Get(); }
		inline ID3D11ShaderResourceView* GetView() { return m_LightBufferView.Get(); }

		bool Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext )
		{
			HYPERION_VERIFY( inDevice != nullptr, "[DX11] Device was null!" );
			HYPERION_VERIFY( inContext != nullptr, "[DX11] Device context was null!" );

			m_Context = inContext;

			// We want to create our buffer based on the size we were constructed with
			D3D11_BUFFER_DESC bufferDesc	= {};
			bufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
			bufferDesc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
			bufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags			= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			bufferDesc.ByteWidth			= sizeof( LightInfo ) * m_MaxLights;
			bufferDesc.StructureByteStride	= sizeof( LightInfo );

			if( FAILED( inDevice->CreateBuffer( &bufferDesc, nullptr, m_LightBuffer.ReleaseAndGetAddressOf() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to initialize light buffer, the buffers couldnt be created" );
				Shutdown();
				return false;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc	= {};
			viewDesc.Format								= DXGI_FORMAT_UNKNOWN;
			viewDesc.ViewDimension						= D3D11_SRV_DIMENSION_BUFFER;
			viewDesc.Buffer.ElementOffset				= 0;
			viewDesc.Buffer.ElementWidth				= sizeof( LightInfo );
			viewDesc.Buffer.FirstElement				= 0;
			viewDesc.Buffer.NumElements					= m_MaxLights;

			if( FAILED( inDevice->CreateShaderResourceView( m_LightBuffer.Get(), &viewDesc, m_LightBufferView.ReleaseAndGetAddressOf() ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to initialize light buffer, the resource view couldnt be created" );
				Shutdown();
				return false;
			}


			return true;
		}


		bool UploadLights( const std::vector< std::shared_ptr< ProxyLight > >& inLights ) final
		{
			HYPERION_VERIFY( m_Context != nullptr, "[DX11] Device context was null!" );

			D3D11_MAPPED_SUBRESOURCE bufferData = {};

			if( FAILED( m_Context->Map( m_LightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData ) ) )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to upload lights to light buffer, it couldnt be mapped into memory!" );
				return false;
			}

			if( inLights.size() == 0 )
			{
				// Size is empty, so just clear out the current buffer
				// When we map it, it should discard previous data and leave it empty
			}
			else
			{
				auto* mappedList = (LightInfo*) bufferData.pData;
				uint32 index = 0;

				for( auto it = inLights.begin(); it != inLights.end(); it++ )
				{
					auto& light_ptr		= *it;
					auto light_pos		= light_ptr->GetTransform().Position;
					auto light_color	= light_ptr->GetColor();

					mappedList[ index ].WorldPosition	= DirectX::XMFLOAT3( light_pos.X, light_pos.Y, light_pos.Z );
					mappedList[ index ].Color			= DirectX::XMFLOAT3( light_color.r, light_color.g, light_color.b );
					mappedList[ index ].Brightness		= light_ptr->GetBrightness();
					mappedList[ index ].AttnRadius		= light_ptr->GetRadius();
					mappedList[ index ].Direction		= DirectX::XMFLOAT3( 0.f, 0.f, 1.f );
					mappedList[ index ].Type			= DX11_LIGHT_TYPE_POINT;
					mappedList[ index ].SpotFOV			= 0.f;

					index++;
				}
			}

			m_Context->Unmap( m_LightBuffer.Get(), 0 );
			m_CurrentLights = (uint32) inLights.size();

			return true;
		}




	};

}