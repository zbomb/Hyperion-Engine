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


	class ProxyScene
	{

	private:

		std::map< ProxyID, std::shared_ptr< ProxyPrimitive > > m_Primitives;
		std::map< ProxyID, std::shared_ptr< ProxyLight > > m_Lights;

		ViewState m_ViewState;
		bool m_bViewStateDirty;

	public:

		ProxyScene();
		~ProxyScene();

		void Initialize();
		void Shutdown();

		bool AddPrimitive( std::shared_ptr< ProxyPrimitive > inPrimitive );
		bool AddLight( std::shared_ptr< ProxyLight > inLight );

		std::shared_ptr< ProxyPrimitive > RemovePrimitive( uint32 Identifier );
		std::shared_ptr< ProxyLight > RemoveLight( uint32 Identifier );

		std::shared_ptr< ProxyPrimitive > FindPrimitive( uint32 Identifier );
		std::shared_ptr< ProxyLight > FindLight( uint32 Identifier );

		void GetViewState( ViewState& outState ) const;
		void UpdateViewState( const ViewState& inState );

	   	void OnCameraUpdate();

	};

}