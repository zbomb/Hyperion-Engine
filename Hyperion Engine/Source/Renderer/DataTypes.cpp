/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DataTypes.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DataTypes.h"
#include "Hyperion/Renderer/Renderer.h"


namespace Hyperion
{

	bool ScreenResolution::LoadFromString( const String& inStr )
	{
		// We want to break it out into two components, seperated by a comma
		String trimmedStr = inStr.TrimBoth();
		std::vector< String > explodedStr = trimmedStr.Explode( ',' );
		
		if( explodedStr.size() != 2 )
		{
			return false;
		}

		uint32 strWidth		= 0;
		uint32 strHeight	= 0;

		if( !explodedStr[ 0 ].ToUInt( strWidth ) ||
			!explodedStr[ 1 ].ToUInt( strHeight ) )
		{
			return false;
		}

		if( strWidth > 420 && strHeight > 360 )
		{
			Width	= strWidth;
			Height	= strHeight;

			return true;
		}
		else
		{
			return false;
		}
	}


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