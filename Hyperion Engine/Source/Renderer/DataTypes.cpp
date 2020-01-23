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


	void AddCameraProxyCommand::Execute( Renderer& inRenderer )
	{
		if( m_Payload )
		{
			inRenderer.AddCamera( m_Payload );
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


	void RemoveCameraProxyCommand::Execute( Renderer& inRenderer )
	{
		inRenderer.RemoveCamera( m_Identifier );
	}
}