/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Renderer.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

/*
*	Headers
*/
#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Core/Types/ConcurrentQueue.h"
#include "Hyperion/Renderer/Commands.h"
#include "Hyperion/Renderer/Misc.h"


namespace Hyperion
{
	/*
	*	Forward Declarations
	*/
	class ProxyScene;
	class ProxyBase;
	class ProxyPrimitive;
	class ProxyLight;
	class RenderResourceManager;

	/*
	*	Renderer Class
	*/
	class Renderer
	{

	protected:

		/*
		*	Data Members
		*/
		std::shared_ptr< ProxyScene > m_Scene;

		/*
		*	Command System Members
		*/
		ConcurrentQueue< std::unique_ptr< RenderCommandBase > > m_ImmediateCommands;
		ConcurrentQueue< std::unique_ptr< RenderCommandBase > > m_Commands;
		std::atomic< bool > m_bAllowCommands;

		/*
		*	Private Functions
		*/
		void ShutdownProxy( const std::shared_ptr< ProxyBase >& inProxy );
		void UpdateScene();

		/*
		*	Protected Virtual Functions
		*/
		virtual void Frame() = 0;

		// These next two functions can be called from outside of the render thread
		virtual bool UpdateResolution( const ScreenResolution& inResolution ) = 0;
		virtual bool UpdateVSync( bool bVSync ) = 0;


	public:

		Renderer();
		virtual ~Renderer() {}

		Renderer( const Renderer& ) = delete;
		Renderer( Renderer&& ) = delete;
		Renderer& operator=( const Renderer& ) = delete;
		Renderer& operator=( Renderer&& ) = delete;

		virtual bool Initialize( void* inWindow, const ScreenResolution& inResolution, bool bVSync );
		virtual void Shutdown();
		virtual void Tick();

		inline std::shared_ptr< ProxyScene > GetScene() const { return m_Scene; }

		virtual bool SetResolution( const ScreenResolution& inResolution ) = 0;
		virtual bool SetVSync( bool bVSync ) = 0;
		virtual ScreenResolution GetResolution() const = 0;
		virtual bool GetVSync() const = 0;

		/*
		*	Proxy Managment
		*/
		bool AddPrimitive( const std::shared_ptr< ProxyPrimitive >& inPrimitive );
		bool AddLight( const std::shared_ptr< ProxyLight >& inLight );
		bool RemovePrimitive( uint32 inIdentifier );
		bool RemoveLight( uint32 inIdentifier );

		/*
		*	Commands
		*/
		void AddImmediateCommand( std::unique_ptr< RenderCommandBase >&& inCommand );
		void AddCommand( std::unique_ptr< RenderCommandBase >&& inCommand );

		/*
		*	Pure Virtual Functions
		*/
		virtual std::shared_ptr< RenderResourceManager > GetResourceManager() const = 0;


	};

}

#define HYPERION_RENDER_COMMAND( _func_ ) Hyperion::Engine::GetRenderer()->AddCommand( std::make_unique< Hyperion::RenderCommand >( _func_, 0 ) )
