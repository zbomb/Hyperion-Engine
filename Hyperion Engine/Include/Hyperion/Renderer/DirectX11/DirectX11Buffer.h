/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/DirectX11Buffer.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Renderer/Resource/Buffer.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


namespace Hyperion
{

	class DirectX11Buffer : public RBuffer
	{

	private:

		ID3D11Buffer* m_Buffer;
		BufferType m_Type;
		uint32 m_Size;
		uint32 m_Count;

		DirectX11Buffer( BufferType inType )
			: m_Buffer( nullptr ), m_Type( inType ), m_Size( 0 )
		{}

	public:

		DirectX11Buffer() = delete;

		DirectX11Buffer( const DirectX11Buffer& inOther )
			: m_Buffer( inOther.m_Buffer ), m_Type( inOther.m_Type ), m_Size( inOther.m_Size )
		{}

		DirectX11Buffer( DirectX11Buffer&& inOther ) noexcept
			: m_Buffer( std::move( inOther.m_Buffer ) ), m_Type( std::move( inOther.m_Type ) ), m_Size( std::move( inOther.m_Size ) )
		{
			inOther.m_Buffer = nullptr;
		}

		virtual ~DirectX11Buffer()
		{
			Shutdown();
		}

		void Shutdown() final
		{
			if( m_Buffer ) { m_Buffer->Release(); }
			m_Buffer = nullptr;
		}

		bool IsValid() const final
		{
			return m_Buffer != nullptr;
		}

		BufferType GetType() const final
		{
			return m_Type;
		}

		uint32 GetSize() const final
		{
			return m_Size;
		}

		void UpdateSize( uint32 inSize )
		{
			m_Size = inSize;
		}

		uint32 GetCount() const final
		{
			return m_Count;
		}

		ID3D11Buffer* GetBuffer()		{ return m_Buffer; }
		ID3D11Buffer** GetAddress()		{ return &m_Buffer; }


		friend class DirectX11Graphics;
	};
}