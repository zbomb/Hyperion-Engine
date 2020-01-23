/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Proxy/ProxyScene.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

// Hyperion Includes
#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/ViewState.h"

// STD Includes
#include <memory>
#include <map>
#include <queue>
#include <mutex>


namespace Hyperion
{
	typedef uint32 ProxyID;

	class ProxyPrimitive;
	class ProxyLight;
	class ProxyCamera;


	class ProxyScene
	{

	private:

		std::map< ProxyID, std::shared_ptr< ProxyPrimitive > > m_Primitives;
		std::map< ProxyID, std::shared_ptr< ProxyLight > > m_Lights;
		std::map< ProxyID, std::shared_ptr< ProxyCamera > > m_Cameras;

		std::shared_ptr< ProxyCamera > m_ActiveCamera;

	public:

		ProxyScene();
		~ProxyScene();

		void Initialize();
		void Shutdown();

		bool AddPrimitive( std::shared_ptr< ProxyPrimitive > inPrimitive );
		bool AddLight( std::shared_ptr< ProxyLight > inLight );
		bool AddCamera( std::shared_ptr< ProxyCamera > inCamera );

		std::shared_ptr< ProxyPrimitive > RemovePrimitive( uint32 Identifier );
		std::shared_ptr< ProxyLight > RemoveLight( uint32 Identifier );
		std::shared_ptr< ProxyCamera > RemoveCamera( uint32 Identifier );

		std::shared_ptr< ProxyPrimitive > FindPrimitive( uint32 Identifier );
		std::shared_ptr< ProxyLight > FindLight( uint32 Identifier );
		std::shared_ptr< ProxyCamera > FindCamera( uint32 Identifier );

		std::shared_ptr< ProxyCamera > GetActiveCamera();
		void SetActiveCamera( std::shared_ptr< ProxyCamera > inPtr );
		void OnCameraUpdate();

	};

}