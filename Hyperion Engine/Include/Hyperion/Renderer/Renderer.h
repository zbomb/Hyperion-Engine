/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Renderer.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/String.h"
#include "Hyperion/Renderer/IGraphics.h"
#include "Hyperion/Renderer/DataTypes.h"
#include "Hyperion/Core/Types/ConcurrentQueue.h"
#include "Hyperion/Console/ConsoleVar.h"
#include "Hyperion/Library/Geometry.h"
#include "Hyperion/Streaming/BasicStreamingManager.h"
#include "Hyperion/Renderer/Resource/ResourceManager.h"

#include <atomic>


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class ProxyScene;
	class ProxyPrimitive;
	class ProxyLight;
	class ProxyCamera;
	class ProxyBase;
	class TextureAsset;

	/*
		Renderer Instance
	*/
	class Renderer
	{

	protected:

		ScreenResolution m_Resolution;
		bool m_bVSync;
		void* m_pWindow;

		std::atomic< ScreenResolution > m_CachedResolution;
		std::atomic< bool > m_bCachedVSync;

		std::vector< ScreenResolution > m_AvailableResolutions;
		std::atomic< bool > m_bCanChangeResolution;

		std::shared_ptr< IGraphics > m_API;
		std::shared_ptr< ProxyScene > m_Scene;

		HypPtr< BasicStreamingManager > m_StreamingManager;

		ConcurrentQueue< std::unique_ptr< RenderCommandBase > > m_ImmediateCommands;
		ConcurrentQueue< std::unique_ptr< RenderCommandBase > > m_Commands;
		std::atomic< bool > m_AllowCommands;

		std::shared_ptr< ResourceManager > m_ResourceManager;

		GraphicsAPI m_APIType;

		// Functions for derived classes to define
		virtual void RenderScene() = 0;
		virtual void OnResolutionChanged( const ScreenResolution& inRes ) = 0;

	public:

		Renderer() = delete;
		Renderer( GraphicsAPI inAPI, void* inWindow, const ScreenResolution& inRes, bool bVSync );
		virtual ~Renderer();

		Renderer( const Renderer& ) = delete;
		Renderer( Renderer&& ) = delete;

		Renderer& operator=( const Renderer& ) = delete;
		Renderer& operator=( Renderer&& ) = delete;

		inline GraphicsAPI GetAPIType() const { return m_APIType; }

		// These two functions are able to be called from any thread
		inline ScreenResolution GetResolutionSafe() const { return m_CachedResolution.load(); }
		inline bool IsVSyncOnSafe() const { return m_bCachedVSync.load(); }

		inline ScreenResolution GetResolutionUnsafe() const { return m_Resolution; }
		inline bool IsVSyncOnUnsafe() const { return m_bVSync; }

		bool ChangeResolution( const ScreenResolution& inRes );

		inline HypPtr< BasicStreamingManager > GetStreamingManager() const { return m_StreamingManager; } // TODO: Make this a normal shared_ptr?
		inline std::shared_ptr< ResourceManager > GetResourceManager() const { return m_ResourceManager; }
		inline std::shared_ptr< IGraphics > GetAPI() const { return m_API; }

		// Derived method requires call to Renderer::Initialize
		virtual bool Initialize();

		// Derived method requires call to Renderer::Shutdown
		// Also, in derived class destructor.. call Shutdown
		virtual void Shutdown();

		void Frame();

		void UpdateScene();
		void ShutdownProxy( const std::shared_ptr< ProxyBase >& );
		void GetViewState( ViewState& outState ) const;

		void AddImmediateCommand( std::unique_ptr< RenderCommandBase >&& inCommand );
		void AddCommand( std::unique_ptr< RenderCommandBase >&& inCommand );

		bool AddPrimitive( std::shared_ptr< ProxyPrimitive >& );
		bool AddLight( std::shared_ptr< ProxyLight >& );

		bool RemovePrimitive( uint32 inIdentifier );
		bool RemoveLight( uint32 inIdentifier );

		inline std::shared_ptr< ProxyScene > GetScene() const { return m_Scene; }
	};

}