/*==================================================================================================
	Hyperion Engine
	Source/Renderer/Proxy/ProxyScene.cpp
	© 2019, Zachary Berry
==================================================================================================*/

// Hyperion Includes
#include "Hyperion/Renderer/Proxy/ProxyScene.h"
#include "Hyperion/Renderer/Proxy/ProxyPrimitive.h"
#include "Hyperion/Renderer/Proxy/ProxyLight.h"
#include "Hyperion/Renderer/Proxy/ProxyCamera.h"

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
		Console::WriteLine( "[DEBUG] ProxyScene: Shutdown..." );

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

		for( auto It = m_Cameras.begin(); It != m_Cameras.end(); It++ )
		{
			if( It->second )
			{
				It->second->BeginShutdown();
				It->second->Shutdown();
			}
		}

		m_Cameras.clear();
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

	bool ProxyScene::AddCamera( std::shared_ptr< ProxyCamera > inCamera )
	{
		if( !inCamera )
		{
			Console::WriteLine( "[WARNING] ProxySystem: Attempt to add null camera to the camera list!" );
			return false;
		}

		auto identifier = inCamera->GetIdentifier();
		auto current_entry = m_Cameras.find( identifier );
		if( current_entry != m_Cameras.end() )
		{
			Console::WriteLine( "[WARNING] ProxySystem: Attempt to add camera with duplicate id" );
			return false;
		}

		m_Cameras[ identifier ] = inCamera;
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

	std::shared_ptr< ProxyCamera > ProxyScene::RemoveCamera( uint32 Identifier )
	{
		auto entry = m_Cameras.find( Identifier );
		if( entry == m_Cameras.end() )
		{
			return nullptr;
		}

		auto copy = entry->second;
		m_Cameras.erase( entry );
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

	std::shared_ptr< ProxyCamera > ProxyScene::FindCamera( uint32 Identifier )
	{
		auto entry = m_Cameras.find( Identifier );
		if( entry == m_Cameras.end() )
		{
			return nullptr;
		}

		return entry->second;
	}


	void ProxyScene::SetActiveCamera( std::shared_ptr< ProxyCamera > inPtr )
	{
		if( m_ActiveCamera != inPtr )
		{
			m_ActiveCamera = inPtr;
			// TODO: Call something?
		}
	}


	std::shared_ptr< ProxyCamera > ProxyScene::GetActiveCamera()
	{
		return m_ActiveCamera;
	}


	void ProxyScene::OnCameraUpdate()
	{
		// TODO
	}

}