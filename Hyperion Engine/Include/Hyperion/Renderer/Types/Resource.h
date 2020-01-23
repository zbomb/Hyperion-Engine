/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Types/Resource.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


namespace Hyperion
{

	class IAPIResourceBase
	{

	public:

		// Ensure only default constructor ever gets called
		IAPIResourceBase() {};
		IAPIResourceBase( const IAPIResourceBase& ) = delete;
		IAPIResourceBase( IAPIResourceBase&& ) = delete;
		IAPIResourceBase& operator=( const IAPIResourceBase& ) = delete;
		IAPIResourceBase& operator=( IAPIResourceBase&& ) = delete;

		virtual ~IAPIResourceBase() {};

		virtual bool IsValid() const = 0;
		virtual void Shutdown() = 0;

	};

}