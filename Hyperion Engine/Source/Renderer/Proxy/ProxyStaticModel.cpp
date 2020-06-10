/*==================================================================================================
	Hyperion Engine
	Source/Renderer/Proxy/ProxyStaticModel.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/Proxy/ProxyStaticModel.h"


namespace Hyperion
{

	ProxyStaticModel::ProxyStaticModel( uint32 inIdentifier )
		: ProxyPrimitive( inIdentifier )
	{}


	void ProxyStaticModel::GameInit()
	{
		if( IsGameThread() )
		{
			Console::WriteLine( "[DEBUG] ProxyStaticModel: Init on game thread as expected!" );
		}
		else
		{
			Console::WriteLine( "[DEBUG] ProxyStaticModel: Game init called on wrong thread!" );
		}
	}


	void ProxyStaticModel::RenderInit()
	{
		if( IsRenderThread() )
		{
			Console::WriteLine( "[DEBUG] ProxyStaticModel: Init on render thread as expected!" );
		}
		else
		{
			Console::WriteLine( "[DEBUG] ProxyStaticModel: Render thread init called on wrong thread!" );
		}
	}


	void ProxyStaticModel::BeginShutdown()
	{
		if( IsRenderThread() )
		{
			Console::WriteLine( "[DEBUG] ProxyStaticModel: Begin shutdown on render thread as expected" );
		}
		else
		{
			Console::WriteLine( "[DEBUG] ProxyStaticModel: Begin shutdown called on wrong thread!" );
		}
	}


	void ProxyStaticModel::Shutdown()
	{
		if( IsWorkerThread() )
		{
			Console::WriteLine( "[DEBUG] ProxyStaticModel: Shutdown on worker thread as expected" );
		}
		else
		{
			Console::WriteLine( "[DEBUG] ProxyStaticModel: Shutdown called on wrong thread!" );
		}
	}

}