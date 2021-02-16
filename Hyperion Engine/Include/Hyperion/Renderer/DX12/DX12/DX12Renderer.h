/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DX12/DX12Renderer.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

/*
*	Headers
*/
#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/DX12/DX12ResourceManager.h"
#include "Hyperion/Renderer/DX12/DX12Core.h"
#include "Hyperion/Renderer/Misc.h"
#include "Hyperion/Renderer/Renderer.h"


namespace Hyperion
{

	class DX12Renderer : public Renderer
	{

	protected:

		/*
		*	Data Members
		*/
		std::shared_ptr< DX12ResourceManager > m_ResourceManager;
		std::shared_ptr< DX12Core > m_Core;

		std::atomic< ScreenResolution > m_Resolution;
		std::atomic< bool > m_bVSync;

		/*
		*	Overriden Functions
		*/
		void Frame() final;

		void OnResolutionChanged();
		void OnVSyncChanged();

	public:

		DX12Renderer();
		~DX12Renderer();

		bool Initialize( void* inWindow, const ScreenResolution& inResolution, bool bVSync ) override;
		void Shutdown() override;

		// These next two functions can be called from any thread
		bool SetResolution( const ScreenResolution& inResolution ) final;
		bool SetVSync( bool bVSync ) final;

		inline ScreenResolution GetResolution() const { return m_Resolution.load(); }
		inline bool GetVSync() const { return m_bVSync.load(); }

		std::shared_ptr< RenderResourceManager > GetResourceManager() const final { return m_ResourceManager; }
		inline std::shared_ptr< DX12Core > GetCore() const { return m_Core; }

	};

}