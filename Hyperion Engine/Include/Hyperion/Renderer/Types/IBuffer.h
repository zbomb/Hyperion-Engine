/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Types/IBuffer.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Types/Resource.h"
#include "Hyperion/Library/Math/Vertex.h"


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
		interface IBuffer
	*/
	class IBuffer : public IAPIResourceBase
	{

	public:

		virtual ~IBuffer() {}
		virtual BufferType GetType() const = 0;

	};
}