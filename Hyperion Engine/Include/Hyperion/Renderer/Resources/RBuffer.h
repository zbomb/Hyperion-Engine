/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resources/RBuffer.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{
	/*
	*	BufferType Enum
	*/
	enum class BufferType
	{
		Vertex			= 1,
		Index			= 2,
		Constant		= 3,
		ShaderResource	= 4,
		StreamOutput	= 5
	};

	/*
	*	Buffer Parameters Structure
	*/
	struct BufferParameters
	{
		BufferType Type;
		bool CanCPURead;
		bool Dynamic;
		uint32 Size;
		uint32 Count;
		const void* Data;

		BufferParameters()
			: Type( BufferType::Vertex ), CanCPURead( false ), Dynamic( false ), Size( 0 ), Count( 0 ), Data( nullptr )
		{}
	};

	/*
	*	Buffer Interface
	*/
	class RBuffer
	{

	public:

		virtual ~RBuffer() {}
		virtual BufferType GetType() const = 0;
		virtual void Shutdown() = 0;
		virtual bool IsValid() const = 0;
		virtual uint32 GetSize() const = 0;
		virtual uint32 GetCount() const = 0;

	};

}