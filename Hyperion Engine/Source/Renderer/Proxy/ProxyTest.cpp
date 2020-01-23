/*==================================================================================================
	Hyperion Engine
	Source/Renderer/Proxy/ProxyTest.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/Proxy/ProxyTest.h"


namespace Hyperion
{

	ProxyTest::ProxyTest( uint32 inIdentifier )
		: ProxyPrimitive( inIdentifier ), m_Value( 0 )
	{}


	void ProxyTest::GameInit()
	{
		if( IsGameThread() )
		{
			Console::WriteLine( "[DEBUG] ProxyTest: Init on game thread as expected!" );
		}
		else
		{
			Console::WriteLine( "[DEBUG] ProxyTest: Game init called on wrong thread!" );
		}
	}


	void ProxyTest::RenderInit()
	{
		if( IsRenderThread() )
		{
			Console::WriteLine( "[DEBUG] ProxyTest: Init on render thread as expected!" );
		}
		else
		{
			Console::WriteLine( "[DEBUG] ProxyTest: Render thread init called on wrong thread!" );
		}
	}


	void ProxyTest::BeginShutdown()
	{
		if( IsRenderThread() )
		{
			Console::WriteLine( "[DEBUG] ProxyTest: Begin shutdown on render thread as expected" );
		}
		else
		{
			Console::WriteLine( "[DEBUG] ProxyTest: Begin shutdown called on wrong thread!" );
		}
	}


	void ProxyTest::Shutdown()
	{
		if( IsWorkerThread() )
		{
			Console::WriteLine( "[DEBUG] ProxyTest: Shutdown on worker thread as expected" );
		}
		else
		{
			Console::WriteLine( "[DEBUG] ProxyTest: Shutdown called on wrong thread!" );
		}
	}

}