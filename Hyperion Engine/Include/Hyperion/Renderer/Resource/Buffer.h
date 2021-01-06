/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resource/Buffer.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{

	/*
		enum BufferType
		- What this buffer will be used for, so the API can optimize
	*/
	enum class BufferType
	{
		Vertex = 1,
		Index = 2,
		Constant = 3,
		ShaderResource = 4,
		StreamOutput = 5
	};


	/*
		structure BufferParameters
	*/
	struct BufferParameters
	{
		BufferType Type;
		bool CanCPURead;
		bool Dynamic;
		uint32 Size;
		const void* Data;

		BufferParameters()
			: Type( BufferType::Vertex ), Dynamic( false ), Size( 0 ), Data( nullptr ), CanCPURead( false )
		{
		}
	};

	/*
		class RBuffer (interface)
	*/
	class RBuffer
	{

	public:

		virtual ~RBuffer() {}
		virtual BufferType GetType() const = 0;
		virtual void Shutdown() = 0;
		virtual bool IsValid() const = 0;

	};

}