/*==================================================================================================
	Hyperion Engine
	Source/Renderer/Proxy/ProxyScene.cpp
	© 2019, Zachary Berry
==================================================================================================*/

// Hyperion Includes
#include "Hyperion/Renderer/Proxy/ProxyScene.h"
#include "Hyperion/Renderer/Proxy/ProxyPrimitive.h"
#include "Hyperion/Renderer/Proxy/ProxyLight.h"

// STD Includes
#include <iostream>

namespace Hyperion
{
	ProxyScene::ProxyScene()
	{

	}


	ProxyScene::~ProxyScene()
	{
		Shutdown();
	}


	/*
		ProxyScene::Initialize
		* Called after the renderer is created
		* Called from main render thread
	*/
	void ProxyScene::Initialize()
	{
		Console::WriteLine( "[DEBUG] ProxyScene: Initializing..." );
	}

	/*
		ProxyScene::Shutdown
		* Called when the renderer is supposed to shutdown
		* Called from main render thread
	*/
	void ProxyScene::Shutdown()
	{
		// Shutdown all of our resources sync
		for( auto It = m_Primitives.begin(); It != m_Primitives.end(); It++ )
		{
			if( It->second )
			{
				It->second->BeginShutdown();
				It->second->Shutdown();
			}
		}

		m_Primitives.clear();

		for( auto It = m_Lights.begin(); It != m_Lights.end(); It++ )
		{
			if( It->second )
			{
				It->second->BeginShutdown();
				It->second->Shutdown();
			}
		}

		m_Lights.clear();
	}


	bool ProxyScene::AddPrimitive( std::shared_ptr< ProxyPrimitive > inPrimitive )
	{
		if( !inPrimitive )
		{
			Console::WriteLine( "[WARNING] ProxySystem: Attempt to add null primitive to the primitive list!" );
			return false;
		}

		auto identifier = inPrimitive->GetIdentifier();
		auto current_entry = m_Primitives.find( identifier );
		if( current_entry != m_Primitives.end() )
		{
			Console::WriteLine( "[WARNING] ProxySystem: Attempt to add primitive with duplicate id!" );
			return false;
		}

		m_Primitives[ identifier ] = inPrimitive;
		return true;
	}

	bool ProxyScene::AddLight( std::shared_ptr< ProxyLight > inLight )
	{
		if( !inLight )
		{
			Console::WriteLine( "[WARNING] ProxySystem: Attempt to add null light to the light list!" );
			return false;
		}

		auto identifier = inLight->GetIdentifier();
		auto current_entry = m_Lights.find( identifier );
		if( current_entry != m_Lights.end() )
		{
			Console::WriteLine( "[WARNING] ProxySystem: Attempt to add light with duplicate id!" );
			return false;
		}

		m_Lights[ identifier ] = inLight;
		return true;
	}


	std::shared_ptr< ProxyPrimitive > ProxyScene::RemovePrimitive( uint32 Identifier )
	{
		auto entry = m_Primitives.find( Identifier );
		if( entry == m_Primitives.end() )
		{
			return nullptr;
		}

		auto copy = entry->second;
		m_Primitives.erase( entry );
		return copy;
	}

	std::shared_ptr< ProxyLight > ProxyScene::RemoveLight( uint32 Identifier )
	{
		auto entry = m_Lights.find( Identifier );
		if( entry == m_Lights.end() )
		{
			return nullptr;
		}

		auto copy = entry->second;
		m_Lights.erase( entry );
		return copy;
	}


	std::shared_ptr< ProxyPrimitive > ProxyScene::FindPrimitive( uint32 Identifier )
	{
		auto entry = m_Primitives.find( Identifier );
		if( entry == m_Primitives.end() )
		{
			return nullptr;
		}

		return entry->second;
	}

	std::shared_ptr< ProxyLight > ProxyScene::FindLight( uint32 Identifier )
	{
		auto entry = m_Lights.find( Identifier );
		if( entry == m_Lights.end() )
		{
			return nullptr;
		}

		return entry->second;
	}


	void ProxyScene::GetViewState( ViewState& outState ) const
	{
		outState = m_ViewState;
	}

	void ProxyScene::UpdateViewState( const ViewState& inState )
	{
		m_ViewState			= inState;
		m_bViewStateDirty	= true;
	}

	void ProxyScene::OnCameraUpdate()
	{
		// TODO
	}

}