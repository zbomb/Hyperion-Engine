/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/DX11Buffer.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Renderer/Resources/RBuffer.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"
#include "Hyperion/Renderer/DirectX11/RDX11Resource.h"


namespace Hyperion
{

	class DX11Buffer : public RBuffer, public RDX11Resource
	{

	private:

		Microsoft::WRL::ComPtr< ID3D11Buffer > m_Buffer;
		Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > m_SRV;
		Microsoft::WRL::ComPtr< ID3D11UnorderedAccessView > m_UAV;

		BufferType m_Type;
		uint32 m_TotalSize;
		uint32 m_ElementSize;
		uint32 m_ElementCount;
		uint32 m_AssetIdentifier;

		DX11Buffer( BufferType inType )
			: m_Buffer( nullptr ), m_SRV( nullptr ), m_UAV( nullptr ), m_Type( inType ), m_TotalSize( 0 ), m_ElementSize( 0 ), m_ElementCount( 0 ), m_AssetIdentifier( ASSET_INVALID )
		{}

	public:

		DX11Buffer() = delete;

		virtual ~DX11Buffer()
		{
			Shutdown();
		}

		void Shutdown() final
		{
			m_UAV.Reset();
			m_SRV.Reset();
			m_Buffer.Reset();
		}

		bool IsValid() const final
		{
			return m_Buffer ? true : false;
		}

		BufferType GetType() const final
		{
			return m_Type;
		}

		uint32 GetSize() const final
		{
			return m_TotalSize;
		}

		uint32 GetElementSize() const final
		{
			return m_ElementSize;
		}

		uint32 GetCount() const final
		{
			return m_ElementCount;;
		}

		uint32 GetAssetIdentifier() const final
		{
			return m_AssetIdentifier;
		}

		ID3D11Buffer* Get()		{ return m_Buffer.Get(); }
		ID3D11Buffer** GetAddress()		{ return m_Buffer.GetAddressOf(); }

		/*
		*	RDX11Resource Implementation
		*/
		inline ID3D11ShaderResourceView* GetSRV() final		{ return m_SRV.Get(); }
		inline ID3D11UnorderedAccessView* GetUAV()	final	{ return m_UAV.Get(); }
		inline ID3D11RenderTargetView* GetRTV() final		{ return nullptr; }
		inline ID3D11ShaderResourceView** GetSRVAddress() final { return m_SRV.GetAddressOf(); }
		inline ID3D11UnorderedAccessView** GetUAVAddress()	final { return m_UAV.GetAddressOf(); }
		inline ID3D11RenderTargetView** GetRTVAddress() final { return nullptr; }
		inline ID3D11Resource* GetResource() final { return m_Buffer.Get(); }
		
		/*
		*	RGraphicsResource Implementation
		*/
		inline bool IsComputeTarget() const final	{ return m_UAV ? true : false; }
		inline bool IsRenderTarget() const final	{ return false; }
		inline bool IsShaderResource() const final	{ return m_SRV ? true : false; }

		friend class DX11Graphics;
	};
}