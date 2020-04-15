/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DataTypes.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DataTypes.h"
#include "Hyperion/Renderer/Renderer.h"


namespace Hyperion
{


	void AddPrimitiveProxyCommand::Execute( Renderer& inRenderer )
	{
		if( m_Payload )
		{
			inRenderer.AddPrimitive( m_Payload );
		}
	}


	void AddLightProxyCommand::Execute( Renderer& inRenderer )
	{
		if( m_Payload )
		{
			inRenderer.AddLight( m_Payload );
		}
	}


	void RemovePrimitiveProxyCommand::Execute( Renderer& inRenderer )
	{
		inRenderer.RemovePrimitive( m_Identifier );
	}


	void RemoveLightProxyCommand::Execute( Renderer& inRenderer )
	{
		inRenderer.RemoveLight( m_Identifier );
	}


	void UpdateViewStateCommand::Execute( Renderer& inRenderer )
	{
		auto scene = inRenderer.GetScene();
		if( !scene )
		{
			Console::WriteLine( "[WARNING] Renderer: Failed to update proxy view state! Proxy scene was null!" );
			return;
		}

		
	}
}