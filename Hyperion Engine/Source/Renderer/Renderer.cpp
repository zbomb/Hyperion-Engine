/*==================================================================================================
	Hyperion Engine
	Source/Renderer/Renderer.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Renderer/Proxy/ProxyScene.h"
#include "Hyperion/Renderer/Proxy/ProxyPrimitive.h"
#include "Hyperion/Renderer/Proxy/ProxyLight.h"
#include "Hyperion/Core/ThreadManager.h"
#include "Hyperion/Core/Asset.h"
#include "Hyperion/Assets/TextureAsset.h"
#include "Hyperion/Library/Math.h"
#include "Hyperion/Renderer/RenderPipeline.h"
#include "Hyperion/Renderer/GBuffer.h"
#include "Hyperion/Renderer/ViewClusters.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Renderer/PostProcessFX.h"
#include "Hyperion/Renderer/Resources/RRenderTarget.h"


// Include active graphics API's, so we can instantitae the current one
#if HYPERION_OS_WIN32
#include "Hyperion/Renderer/DirectX11/DirectX11Graphics.h"
#endif


namespace Hyperion
{

	/*
	*	Console Variables
	*/
	ConsoleVar< String > g_CVar_AntiAliasing = ConsoleVar< String >(
		"r_anti_aliasing", "The type of anti-aliasing to use", "none",
		[] ( const String& inNewSetting )
		{
			auto renderer = Engine::GetRenderer();
			if( renderer ) { renderer->OnAntiAliasSettingChanged( StrToAAType( inNewSetting ) ); }
		}, THREAD_RENDERER );

	ConsoleVar< uint32 > g_CVar_DynamicShadowMaxQuality = ConsoleVar< uint32 >(
		"r_dynamic_shadow_max_quality", "The maximum level of quality to use when rendering dynamic shadows, 0 is highest [8k], 7 is lowest",
		1, 0, 7, [] ( uint32 inNewSetting )
		{
			auto renderer = Engine::GetRenderer();
			if( renderer ) { renderer->OnShadowQualityChanged( inNewSetting ); }
		}, THREAD_RENDERER );

	ConsoleVar< uint32 > g_CVar_DynamicShadowMemoryPoolSize = ConsoleVar< uint32 >(
		"r_dynamic_shadow_memory_pool_size", "The amount of memory (in MB) to use for the shadow map memory pool",
		128, 8, 32768, [] ( uint32 inNewSetting )
		{
			auto renderer = Engine::GetRenderer();
			if( renderer ) { renderer->OnShadowMemoryPoolSizeChanged( inNewSetting ); }
		}, THREAD_RENDERER );

	ConsoleVar< uint32 > g_CVar_DynamicShadowLimit = ConsoleVar< uint32 >(
		"r_dynamic_shadow_limit", "The most number of dynamic shadows that can be rendered in a frame (exluding directional light shadow)",
		16, 0, 128, [] ( uint32 inNewSetting )
		{
			auto renderer = Engine::GetRenderer();
			if( renderer ) { renderer->OnDynamicShadowLimitChanged( inNewSetting ); }
		}, THREAD_RENDERER );


	/*
	*	Constructor
	*/
	Renderer::Renderer( GraphicsAPI inAPI, void* inWindow, const ScreenResolution& inRes, bool bVSync )
		: m_APIType( inAPI ), m_pWindow( inWindow ), m_Resolution( inRes ), m_bVSync( bVSync ), m_Scene( std::make_shared< ProxyScene >() ), m_AllowCommands( true ),
		m_AmbientLightColor( 1.f, 1.f, 1.f ), m_AmbientLightIntensity( 0.2f ), m_AAType( AntiAliasingType::None ), m_LastFramePresent( std::chrono::high_resolution_clock::now() ),
		m_AverageFramesPerSecond( 0.f )
	{
		// We store a copy of the resolution info in an atomic variable
		// This way we can get this info from other threads without a data race
		m_CachedResolution.store( inRes );
		m_bCachedVSync.store( bVSync );

		Console::WriteLine( "[Renderer] Starting renderer at a resolution of ", inRes.Width, "x", inRes.Height, " and in ", inRes.FullScreen ? "fullscreen" : "windowed", " mode" );

		// Determine which type of AA to use
		m_AAType = StrToAAType( g_CVar_AntiAliasing.GetValue() );

		// Create the adaptive asset manaer
		m_StreamingManager	= CreateObject< BasicStreamingManager >();
		m_ResourceManager	= std::make_shared< ResourceManager >();

		// Create the correct API type
		switch( inAPI )
		{
		case GraphicsAPI::DX11:
			m_API = std::make_shared< DirectX11Graphics >();
			break;
		case GraphicsAPI::DX12:
			HYPERION_NOT_IMPLEMENTED( "DirectX 12 not yet impelemented" );
			break;
		case GraphicsAPI::OpenGL:
			HYPERION_NOT_IMPLEMENTED( "OpenGL not yet implemented" );
			break;
		default:
			HYPERION_VERIFY( true, "[Renderer] Invalid graphics API type!" );
			break;
		}

		// Tell the graphics api about the resolution and other settings
		m_API->SetResolution( inRes );
		m_API->SetVSync( bVSync );
	}

	/*
	*	Destructor
	*/
	Renderer::~Renderer()
	{
	}

	void Renderer::OnAntiAliasSettingChanged( AntiAliasingType inSetting )
	{

	}

	void Renderer::OnShadowQualityChanged( uint32 inSetting )
	{
	}

	void Renderer::OnShadowMemoryPoolSizeChanged( uint32 inSetting )
	{
	}

	void Renderer::OnDynamicShadowLimitChanged( uint32 inSetting )
	{
	}

	/*
	*	Renderer::Initialize
	*/
	bool Renderer::Initialize()
	{
		HYPERION_VERIFY( m_API, "[Renderer] API was null during call to 'Initialize'" );

		// Initialize the graphics api
		if( !m_API->Initialize( m_pWindow ) )
		{
			// The api would have printed an error message giving info on why init failed
			return false;
		}

		// Cache a list of available resolutions
		m_AvailableResolutions	= m_API->GetAvailableResolutions();
		m_bCanChangeResolution	= true;

		// Iniitalize the scene
		m_Scene->Initialize();
		m_API->CalculateScreenViewMatrix( m_ScreenViewMatrix );

		// Create compute shaders
		m_BuildClustersShader	= m_API->CreateComputeShader( ComputeShaderType::BuildClusters );
		m_FindClustersShader	= m_API->CreateComputeShader( ComputeShaderType::FindClusters );
		m_CullLightsShader		= m_API->CreateComputeShader( ComputeShaderType::CullLights );

		if( !m_BuildClustersShader || !m_FindClustersShader || !m_CullLightsShader )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to create compute shaders!" );
			return false;
		}

		// Create light buffer and view clusters
		m_LightBuffer	= m_API->CreateLightBuffer();
		m_ViewClusters	= m_API->CreateViewClusters();

		if( !m_LightBuffer || !m_ViewClusters )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to initialize renderer, couldnt create render resources needed" );
			return false;
		}

		// Ensure view clusters get built on the first frame
		m_ViewClusters->MarkDirty();

		// Create resources needed for post-processing system
		TextureParameters params {};

		params.AssetIdentifier	= ASSET_INVALID;
		params.bAutogenMips		= false;
		params.bCPURead			= false;
		params.bDynamic			= false;
		params.BindTargets		= RENDERER_TEXTURE_BIND_FLAG_SHADER | RENDERER_TEXTURE_BIND_FLAG_RENDER;
		params.Format			= TextureFormat::RGBA_8BIT_UNORM_SRGB;
		params.Width			= m_Resolution.Width;
		params.Height			= m_Resolution.Height;
		params.Depth			= 1;
		
		m_PostProcessBackBuffer		= m_API->CreateTexture2D( params );
		m_PostProcessFrontBuffer	= m_API->CreateTexture2D( params );
		m_PostProcessBackTarget		= m_API->CreateRenderTarget( m_PostProcessBackBuffer );
		m_PostProcessFrontTarget	= m_API->CreateRenderTarget( m_PostProcessFrontBuffer );

		if( !m_PostProcessBackBuffer || !m_PostProcessFrontBuffer || !m_PostProcessBackBuffer->IsValid() || !m_PostProcessFrontBuffer->IsValid() ||
			!m_PostProcessBackTarget || !m_PostProcessFrontTarget || !m_PostProcessBackTarget->IsValid() || !m_PostProcessFrontTarget->IsValid() )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to initialize renderer, couldnt create post-process resources" );
			return false;
		}

		m_PostProcessVertexShader = m_API->CreateVertexShader( VertexShaderType::Screen );
		if( !m_PostProcessVertexShader || !m_PostProcessVertexShader->IsValid() )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to initialize renderer, couldnt create post-process vertex shader" );
			return false;
		}

		return true;
	}

	/*
	*	Renderer::Shutdown
	*/
	void Renderer::Shutdown()
	{
		// Kill the command queue
		// TODO: Add a 'locking' feature to ConcurrentQueue, where we can stop new entries from being added
		m_AllowCommands = false;
		std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

		m_Commands.Clear();
		m_ImmediateCommands.Clear();

		m_PostProcessFrontBuffer.reset();
		m_PostProcessBackBuffer.reset();
		m_PostProcessFrontTarget.reset();
		m_PostProcessBackTarget.reset();

		m_PostProcessVertexShader.reset();

		m_BuildClustersShader.reset();
		m_FindClustersShader.reset();
		m_CullLightsShader.reset();

		m_LightBuffer.reset();
		m_ViewClusters.reset();
		m_GBuffer.reset();

		DetachPipeline();
		m_ResourceManager.reset();
		if( m_StreamingManager ) { DestroyObject( m_StreamingManager ); }
		m_StreamingManager.Clear();
		m_Scene.reset();
		m_API.reset();
	}

	/*
	*	Renderer::ChangeResolution
	*/
	bool Renderer::ChangeResolution( const ScreenResolution& inRes )
	{
		// Check if we have started up and cached the list of available resolutions
		if( !m_bCanChangeResolution )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to change resolution.. renderer hasnt finished init yet!" );
			return false;
		}

		// Check if the target resolution is supported
		if( inRes.Width < MIN_RESOLUTION_WIDTH || inRes.Height < MIN_RESOLUTION_HEIGHT )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to change resolution.. target resolution is lower than the minimum" );
			return false;
		}

		bool bValid = false;
		for( auto it = m_AvailableResolutions.begin(); it != m_AvailableResolutions.end(); it++ )
		{
			if( inRes.Width == ( *it ).Width && inRes.Height == ( *it ).Height )
			{
				bValid = true;
				break;
			}
		}

		if( !bValid )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to change resolution.. target resolution is not supported" );
			return false;
		}

		// Add a render command to change the resolution, this way we can ensure it happens between render passes
		AddCommand( std::make_unique< RenderCommand >(
			[ inRes ] ( Renderer& r )
			{
				// Tell the API to update the resolution
				if( !r.m_API->SetResolution( inRes ) )
				{
					Console::WriteLine( "[Warning] Renderer: Failed to change resolution.. API call failed" );
					return;
				}
				
				// Cache the resolution
				r.m_CachedResolution.store( inRes );

				// Update GBuffer Resolution
				if( r.m_GBuffer ) { r.m_GBuffer->UpdateDimensions( r.m_API, inRes.Width, inRes.Height ); }

				// Mark View Clusters Dirty
				if( r.m_ViewClusters ) { r.m_ViewClusters->MarkDirty(); }

				// Update post-processing resources
				r.m_PostProcessFrontBuffer.reset();
				r.m_PostProcessBackBuffer.reset();
				r.m_PostProcessFrontTarget.reset();
				r.m_PostProcessBackTarget.reset();

				TextureParameters params {};

				params.AssetIdentifier	= ASSET_INVALID;
				params.bAutogenMips		= false;
				params.bCPURead			= false;
				params.bDynamic			= false;
				params.BindTargets		= RENDERER_TEXTURE_BIND_FLAG_SHADER | RENDERER_TEXTURE_BIND_FLAG_RENDER;
				params.Format			= TextureFormat::RGBA_8BIT_UNORM_SRGB;
				params.Width			= inRes.Width;
				params.Height			= inRes.Height;
				params.Depth			= 1;
				
				r.m_PostProcessFrontBuffer	= r.m_API->CreateTexture2D( params );
				r.m_PostProcessBackBuffer	= r.m_API->CreateTexture2D( params );
				r.m_PostProcessFrontTarget	= r.m_API->CreateRenderTarget( r.m_PostProcessFrontBuffer );
				r.m_PostProcessBackTarget	= r.m_API->CreateRenderTarget( r.m_PostProcessBackBuffer );

				if( !r.m_PostProcessFrontBuffer || !r.m_PostProcessBackBuffer || !r.m_PostProcessFrontTarget || !r.m_PostProcessBackTarget ||
					!r.m_PostProcessFrontBuffer->IsValid() || !r.m_PostProcessBackBuffer->IsValid() || !r.m_PostProcessFrontTarget->IsValid() || !r.m_PostProcessBackTarget->IsValid() )
				{
					Console::WriteLine( "[ERROR] Renderer: Failed to update post-processing resources for resolution update!" );
				}

				// Call OnResolutionUpdated
				r.OnResolutionChanged( inRes );

				Console::WriteLine( "Renderer: Resolution changed to ", inRes.Width, "x", inRes.Height, " with fullscreen ", ( inRes.FullScreen ? "enabled" : "disabled" ) );
			} )
		);

		return true;
	}

	/*
	*	Renderer::Frame
	*/
	void Renderer::Frame()
	{
		// Update our 'proxy' scene, and store the current view state
		//auto frameBegin = std::chrono::high_resolution_clock::now();
		if( !UpdateScene() )
		{
			// If we couldnt consume an entire frame of updates, we end the frame early before drawing
			// This might cause some uneven frame rates though, when the renderer and game thread are running at different frequencies
			return;
		}

		//auto updateEnd = std::chrono::high_resolution_clock::now();

		GetViewState( m_ViewState );

		// Update our matricies
		m_API->CalculateViewMatrix( m_ViewState, m_ViewMatrix );
		m_API->CalculateProjectionMatrix( m_Resolution, m_ViewState.FOV, m_ProjectionMatrix );
		m_API->CalculateOrthoMatrix( m_Resolution, m_OrthoMatrix );

		m_API->SetCameraInfo( m_ViewState ); // TODO: DEPRECATE THIS
		//auto matrixEnd = std::chrono::high_resolution_clock::now();

		// Next, we need to prepare the API for the frame
		m_API->BeginFrame();
		//auto beginEnd = std::chrono::high_resolution_clock::now();

		// Call derived class to render the current scene
		RenderScene();
		//auto renderEnd = std::chrono::high_resolution_clock::now();

		// Then, we need to render post-process FX, like FXAA, etc..
		RenderPostProcessFX();
		//auto postEnd = std::chrono::high_resolution_clock::now();

		// Calculate the average FPS over the last 60 frames
		auto prePresentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration< float, std::milli > frameTimeDur = prePresentTime - m_LastFramePresent;
		float frameTime = frameTimeDur.count();

		// Calculate average frame time, over the last 60 frames
		auto frameCount		= m_PreviousFrameTimes.size();
		float avgFrameTime	= 0.f;

		for( int i = 0; i < frameCount; i++ )
		{
			avgFrameTime += m_PreviousFrameTimes[ i ];
		}

		avgFrameTime /= static_cast<float>( frameCount );
		m_AverageFramesPerSecond.store( 1000.f / avgFrameTime );

		m_PreviousFrameTimes.push_back( frameTime );
		
		if( frameCount > 60 )
		{
			m_PreviousFrameTimes.pop_front();
		}

		m_LastFramePresent = prePresentTime;
		m_API->EndFrame();

		//auto endEnd = std::chrono::high_resolution_clock::now();

		//std::chrono::duration< double, std::micro > updateTime = updateEnd - frameBegin;
		//std::chrono::duration< double, std::micro > matrixTime = matrixEnd - updateEnd;
		//std::chrono::duration< double, std::micro > beginTime = beginEnd - matrixEnd;
		//std::chrono::duration< double, std::micro > renderTime = renderEnd - matrixEnd;
		//std::chrono::duration< double, std::micro > postTime = postEnd - renderEnd;
		//std::chrono::duration< double, std::micro > endTime = endEnd - postEnd;
		//std::chrono::duration< double, std::micro > frameTime = endEnd - frameBegin;

		//Console::WriteLine( "----------------------------------------------------------------------------------- \nScene Update: ", updateTime.count(), "us \nMatrix Calculation: ", matrixTime.count(), "us \nFrame Begin: ", beginTime.count(),
		//					"us \nRender: ", renderTime.count(), "us \nPost Process: ", postTime.count(), "us \nFrame End: ", endTime.count(), "us\nTotal: ", frameTime.count(), "us" );
	}

	/*
	*	Renderer::UpdateScene
	*/
	bool Renderer::UpdateScene()
	{
		// Execute all immediate commands
		// Were going to run them until the list is empty
		auto nextImmediateCommand = m_ImmediateCommands.PopValue();
		while( nextImmediateCommand.first )
		{
			// Execute
			if( nextImmediateCommand.second )
			{
				nextImmediateCommand.second->Execute( *this );
			}

			// Pop next command
			nextImmediateCommand = m_ImmediateCommands.PopValue();
		}

		// For now, were just going to run the next frame of commands
		auto nextCommand	= m_Commands.PopValue();

		while( nextCommand.first )
		{
			// Execute this command
			if( nextCommand.second )
			{
				nextCommand.second->Execute( *this );
			}

			// Check if this is the end of the frame, by the EOF flag
			if( nextCommand.second->HasFlag( RENDERER_COMMAND_FLAG_END_OF_FRAME ) )
			{
				return true;
			}

			// Pop the next command in the list
			nextCommand = m_Commands.PopValue();
		}

		// Were going to break out of the tick function, and let it execute again
		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		return false;
	}

	/*
	*	Renderer::AddImmediateCommand
	*/
	void Renderer::AddImmediateCommand( std::unique_ptr< RenderCommandBase >&& inCommand )
	{
		if( !m_AllowCommands ) 
		{ 
			Console::WriteLine( "[Warning] Renderer: Failed to push immediate command, the queue was closed" ); 
			return; 
		}

		m_ImmediateCommands.Push( std::move( inCommand ) );
	}

	/*
	*	Renderer::AddCommand
	*/
	void Renderer::AddCommand( std::unique_ptr< RenderCommandBase >&& inCommand )
	{
		if( !m_AllowCommands )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to push command, the queue was closed" );
			return;
		}

		m_Commands.Push( std::move( inCommand ) );
	}

	/*
	*	Renderer::AddPrimitive
	*/
	bool Renderer::AddPrimitive( std::shared_ptr< ProxyPrimitive >& inPrimitive )
	{
		if( !m_Scene )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to add primitive.. scene was null!" );
			return false;
		}

		// Check if a primitive already exists with this identifier
		auto ptr = m_Scene->RemovePrimitive( inPrimitive->GetIdentifier() );
		if( ptr )
		{
			ShutdownProxy( ptr );
		}

		if( m_Scene->AddPrimitive( inPrimitive ) )
		{
			inPrimitive->RenderInit();
			return true;
		}
		else
		{
			return false;
		}
	}

	/*
	*	Renderer::AddLight
	*/
	bool Renderer::AddLight( std::shared_ptr< ProxyLight >& inLight )
	{
		if( !m_Scene )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to add light.. scene was null!" );
			return false;
		}

		auto ptr = m_Scene->RemoveLight( inLight->GetIdentifier() );
		if( ptr )
		{
			ShutdownProxy( ptr );
		}

		if( m_Scene->AddLight( inLight ) )
		{
			inLight->RenderInit();
			return true;
		}
		else
		{
			return false;
		}
	}

	/*
	*	Renderer::ShutdownProxy
	*/
	void Renderer::ShutdownProxy( const std::shared_ptr< ProxyBase >& inProxy )
	{
		if( inProxy )
		{
			// Call begin shutdown directly on render thread
			inProxy->BeginShutdown();

			// Create task on pool to finalize shutdown of the proxy so we dont block the render thread
			ThreadManager::CreateTask< void >(
				[ inProxy ] ()
				{
					if( inProxy )
					{
						inProxy->Shutdown();
					}

				} );
		}
	}

	/*
	*	Renderer::RemovePrimitive
	*/
	bool Renderer::RemovePrimitive( uint32 inIdentifier )
	{
		if( !m_Scene )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to remove primitive.. scene was null!" );
			return false;
		}

		auto ptr = m_Scene->RemovePrimitive( inIdentifier );
		if( ptr )
		{
			ShutdownProxy( ptr );
			return true;
		}
		else
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to remove primitive.. couldnt find! Id = ", inIdentifier );
			return false;
		}
	}

	/*
	*	Renderer::RemoveLight
	*/
	bool Renderer::RemoveLight( uint32 inIdentifier )
	{
		if( !m_Scene )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to remove light.. scene was null!" );
			return false;
		}

		auto ptr = m_Scene->RemoveLight( inIdentifier );
		if( ptr )
		{
			ShutdownProxy( ptr );
			return true;
		}

		return false;
	}

	/*
	*	Renderer::GetViewState
	*/
	void Renderer::GetViewState( ViewState& outState ) const
	{
		if( m_Scene )
		{
			m_Scene->GetViewState( outState );
		}
		else
		{
			// Return a default view state
			outState.FOV = Math::PIf / 4.f;
			outState.Position.Clear();
			outState.Rotation.Clear();
		}
	}


	bool Renderer::AttachPipeline( const std::shared_ptr< RenderPipeline >& inPipeline )
	{
		// Ensure the pipeline isnt null, and is valid
		if( !inPipeline || !inPipeline->IsValid() )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to attach pipeline, it was null!" );
			return false;
		}

		// Check if there is a pipeline already attached
		if( m_AttachedPipeline )
		{
			if( m_AttachedPipeline->IsValid() )
			{
				Console::WriteLine( "[Warning] Renderer: Attaching a pipeline, without detaching the previous pipeline first! Forcing previous pipeline to detach" );
				DetachPipeline();
			}
			else
			{
				m_AttachedPipeline = nullptr;
			}
		}

		// Call attach on all of the shaders
		auto vertexShader		= inPipeline->GetVertexShader();
		auto geometryShader		= inPipeline->GetGeometryShader();
		auto pixelShader		= inPipeline->GetPixelShader();

		HYPERION_VERIFY( vertexShader && pixelShader, "[Renderer] Pipeline is valid, but vertex and/or pixel shader null" );

		if( !vertexShader->Attach() )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to attach pipeline, vertex shader failed to attach" );
			return false;
		}

		if( geometryShader && !geometryShader->Attach() )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to attach pipeline, geometry shader failed to attach" );
			return false;
		}

		if( !pixelShader->Attach() )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to attach pipeline, pixel shader failed to attach" );
			return false;
		}

		m_AttachedPipeline = inPipeline;
		return true;
	}


	void Renderer::DetachPipeline()
	{
		if( m_AttachedPipeline )
		{
			auto vertexShader		= m_AttachedPipeline->GetVertexShader();
			auto geometryShader		= m_AttachedPipeline->GetGeometryShader();
			auto pixelShader		= m_AttachedPipeline->GetPixelShader();

			HYPERION_VERIFY( vertexShader && pixelShader, "[Renderer] Vertex and/or pixel shader is null" );

			// Detach shaders
			pixelShader->Detach();
			if( geometryShader ) { geometryShader->Detach(); }
			vertexShader->Detach();

			m_AttachedPipeline.reset();
		}
	}


	uint32 Renderer::DispatchPipeline( uint32 inFlags /* = 0 */ )
	{
		// We can only use this function to dispatch piplines that render the screen quad
		if( !m_AttachedPipeline || !m_AttachedPipeline->IsValid() || m_AttachedPipeline->GetCollectionSource() != GeometryCollectionSource::ScreenQuad )
		{
			Console::WriteLine( "[ERROR] Renderer: Attempt to dispatch pipeline that requires scene geometry, without providing any geometry!" );
			return 0;
		}

		BatchCollector collector;
		return DispatchPipeline( collector, inFlags );
	}


	uint32 Renderer::DispatchPipeline( const BatchCollector& inBatches, uint32 inFlags /* = 0 */ )
	{
		HYPERION_VERIFY( m_Scene, "[Renderer] Scene was null!" );

		// Ensure current pipeline is valid
		if( !m_AttachedPipeline || !m_AttachedPipeline->IsValid() )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to dispatch pipeline, the attached pipeline isnt valid" );
			return 0;
		}

		// Get the shaders
		auto vertexShader		= m_AttachedPipeline->GetVertexShader();
		auto geometryShader		= m_AttachedPipeline->GetGeometryShader();
		auto pixelShader		= m_AttachedPipeline->GetPixelShader();

		HYPERION_VERIFY( vertexShader && pixelShader, "[Renderer] Vertex and/or pixel shader was null" );

		// Read some parameters
		bool bUseGBuffer		= m_AttachedPipeline->IsGBufferEnabled();
		bool bUseClusters		= m_AttachedPipeline->IsViewClustersEnabled();
		bool bUseLighting		= m_AttachedPipeline->IsLightBufferEnabled();
		auto renderTarget		= m_AttachedPipeline->GetRenderTarget();
		auto collectionSource	= m_AttachedPipeline->GetCollectionSource();
		auto collectionFlags	= m_AttachedPipeline->GetCollectionFlags();

		if( bUseClusters && !m_ViewClusters )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to dispatch pipeline, requires use of view clusters, but the view clusters are null!" );
			return 0;
		}

		if( bUseLighting && !m_LightBuffer )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to dispatch pipeline, requires use of light buffer, but the light buffer is null!" );
			return 0;
		}

		// TODO: Its not ideal to have the GBuffer stored in the base renderer class..
		// Although it is optional to have a GBuffer set in the renderer, it doesnt feel right
		if( bUseGBuffer && !m_GBuffer )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to dispatch pipeline, requires a GBuffer, but there is no GBuffer!" );
			return 0;
		}

		//auto uploadBegin = std::chrono::high_resolution_clock::now();

		// Upload static parameters
		if( !vertexShader->UploadStaticParameters( *this, inFlags ) ||
			!pixelShader->UploadStaticParameters( *this, inFlags ) ||
			( geometryShader && !geometryShader->UploadStaticParameters( *this, inFlags ) ) )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to dispatch pipeline, static parameter upload failed" );
			return 0;
		}

		//auto uploadEnd = std::chrono::high_resolution_clock::now();
		//std::chrono::duration< double, std::nano > uploadTime = uploadEnd - uploadBegin;

		// Upload g-buffer
		if( bUseGBuffer )
		{
			if( !vertexShader->UploadGBuffer( *m_GBuffer ) ||
				!pixelShader->UploadGBuffer( *m_GBuffer ) ||
				( geometryShader && !geometryShader->UploadGBuffer( *m_GBuffer ) ) )
			{
				Console::WriteLine( "[ERROR] Renderer: Failed to dispatch pipeline, couldnt upload GBuffer" );
				return 0;
			}
		}

		//auto gbufferEnd = std::chrono::high_resolution_clock::now();
		//std::chrono::duration< double, std::nano > gbufferTime = gbufferEnd - uploadEnd;

		// Upload view clusters
		if( bUseClusters )
		{
			if( !vertexShader->UploadViewClusters( *m_ViewClusters ) ||
				!pixelShader->UploadViewClusters( *m_ViewClusters ) ||
				( geometryShader && !geometryShader->UploadViewClusters( *m_ViewClusters ) ) )
			{
				Console::WriteLine( "[ERROR] Renderer: Failed to dispatch pipeline, couldnt upload view clusters" );
				return 0;
			}
		}

		//auto clusterEnd = std::chrono::high_resolution_clock::now();
		//std::chrono::duration< double, std::nano > clusterTime = clusterEnd - gbufferEnd;

		// Upload light buffer, light buffer is updated in the pre-pass phase
		if( bUseLighting )
		{
			if( !vertexShader->UploadLightBuffer( *m_LightBuffer ) ||
				!pixelShader->UploadLightBuffer( *m_LightBuffer ) ||
				( geometryShader && !geometryShader->UploadLightBuffer( *m_LightBuffer ) ) )
			{
				Console::WriteLine( "[ERROR] Renderer: Failed to dispatch pipeline, couldnt upload light buffer" );
				return 0;
			}
		}

		//auto lightingEnd = std::chrono::high_resolution_clock::now();
		//std::chrono::duration< double, std::nano > lightingTime = lightingEnd - clusterEnd;

		// Get the target depth stencil
		std::shared_ptr< RDepthStencil > depthStencil {};

		switch( m_AttachedPipeline->GetDepthStencilTarget() )
		{
		case PipelineDepthStencilTarget::Screen:
			depthStencil = m_API->GetDepthStencil();
			break;
		case PipelineDepthStencilTarget::GBuffer:
			depthStencil = m_GBuffer->GetDepthStencil();
			break;
		}

		// Clear the depth stencil if needed
		if( m_AttachedPipeline->IsDepthBufferClearingEnabled() && m_AttachedPipeline->GetDepthStencilTarget() != PipelineDepthStencilTarget::None )
		{
			m_API->ClearDepthStencil( depthStencil, Color4F( 0.f, 0.f, 0.f, 1.f ) );
		}

		// Attach and clear render target
		switch( m_AttachedPipeline->GetRenderTarget() )
		{
		case PipelineRenderTarget::BackBuffer:
			// Clear the render target if needed
			if( m_AttachedPipeline->IsRenderTargetClearingEnabled() )
			{
				m_API->ClearRenderTarget( m_API->GetRenderTarget(), Color4F( 0.f, 0.f, 0.f, 1.f ) );
			}

			m_API->SetRenderTarget( m_API->GetRenderTarget(), depthStencil );
			break;

		case PipelineRenderTarget::GBuffer:

			// Clear render targets if needed
			if( m_AttachedPipeline->IsRenderTargetClearingEnabled() )
			{
				m_GBuffer->ClearRenderTargets( m_API );
			}

			m_API->SetGBufferRenderTarget( m_GBuffer, depthStencil );
			break;

		case PipelineRenderTarget::ViewClusters: // TODO: Deprecate this if we can

			m_API->SetNoRenderTargetAndClusterWriteAccess( m_ViewClusters );
			break;

		case PipelineRenderTarget::PostProcessBuffer:
		{
			auto target = m_bPostProcessFrontTarget ? m_PostProcessBackTarget : m_PostProcessFrontTarget;

			if( m_AttachedPipeline->IsRenderTargetClearingEnabled() )
			{
				m_API->ClearRenderTarget( target, Color4F( 0.f, 0.f, 0.f, 0.f ) );
			}

			m_API->SetRenderTarget( target, depthStencil );
			break;
		}
		}

		//auto targetEnd = std::chrono::high_resolution_clock::now();
		//std::chrono::duration< double, std::nano > targetTime = targetEnd - lightingEnd;

		// Disable Z-Buffer if need be
		if( m_AttachedPipeline->IsZBufferEnabled() )
		{
			m_API->EnableZBuffer();
		}
		else
		{
			m_API->DisableZBuffer();
		}

		if( m_AttachedPipeline->IsAlphaBlendingEnabled() )
		{
			m_API->EnableAlphaBlending();
		}
		else
		{
			m_API->DisableAlphaBlending();
		}

		// Render geometry based on the collection source supplied
		uint32 batchCount = 0;

		//auto zbEnd = std::chrono::high_resolution_clock::now();
		//std::chrono::duration< double, std::nano > zbTime = zbEnd - targetEnd;

		if( collectionSource == GeometryCollectionSource::Scene )
		{

			if( HYPERION_HAS_FLAG( collectionFlags, RENDERER_GEOMETRY_COLLECTION_FLAG_OPAQUE ) )
			{
				for( auto it = inBatches.m_OpaqueGroups.begin(); it != inBatches.m_OpaqueGroups.end(); it++ )
				{
					// If there are no batches in this group, then skip (shouldnt happen)
					if( it->second.Batches.size() == 0 || it->second.IndexBuffer == nullptr || it->second.VertexBuffer == nullptr ) { continue; }

					// Upload this geometry to the graphics API
					m_API->UploadGeometry( it->second.IndexBuffer, it->second.VertexBuffer );

					// Iterate through the groups of different materials
					for( auto mit = it->second.Batches.begin(); mit != it->second.Batches.end(); mit++ )
					{
						if( mit->second.Material == nullptr ) { continue; }

						// Upload the material to the shaders
						if( !vertexShader->UploadBatchMaterial( *mit->second.Material ) ||
							!pixelShader->UploadBatchMaterial( *mit->second.Material ) ||
							( geometryShader && !geometryShader->UploadBatchMaterial( *mit->second.Material ) ) )
						{
							continue;
						}

						// Now we need to render the instances in groups of 512
						uint32 instanceCount = (uint32)mit->second.InstanceTransforms.size();
						uint32 groupCount = ( ( instanceCount - 1 ) / RENDERER_MAX_INSTANCES_PER_BATCH ) + 1;

						for( uint32 i = 0; i < groupCount; i++ )
						{
							uint32 groupBegin = i * RENDERER_MAX_INSTANCES_PER_BATCH;
							uint32 groupEnd = Math::Min( groupBegin + RENDERER_MAX_INSTANCES_PER_BATCH, (uint32) instanceCount );
							uint32 groupSize = groupEnd - groupBegin;

							// Now we know how many instances are in this group, and the range in the list to look for the transforms
							// So, we need to upload a transform list to the shaders if they need it
							std::vector< Matrix > matrixList;
							for( uint32 j = groupBegin; j < groupEnd; j++ )
							{
								matrixList.push_back( mit->second.InstanceTransforms[ j ] );
							}

							if( !vertexShader->UploadBatchTransforms( matrixList ) ||
								!pixelShader->UploadBatchTransforms( matrixList ) ||
								( geometryShader && !geometryShader->UploadBatchTransforms( matrixList ) ) )
							{
								continue;
							}

							batchCount += groupSize;

							// Render the batch
							m_API->RenderBatch( groupSize, it->second.IndexBuffer->GetCount() );
						}
					}
				}
			}

			if( HYPERION_HAS_FLAG( collectionFlags, RENDERER_GEOMETRY_COLLECTION_FLAG_TRANSLUCENT ) )
			{
				for( auto it = inBatches.m_TranslucentGroups.begin(); it != inBatches.m_TranslucentGroups.end(); it++ )
				{
					// If there are no batches in this group, then skip (shouldnt happen)
					if( it->second.Batches.size() == 0 || it->second.IndexBuffer == nullptr || it->second.VertexBuffer == nullptr ) { continue; }

					// Upload this geometry to the graphics API
					m_API->UploadGeometry( it->second.IndexBuffer, it->second.VertexBuffer );

					// Iterate through the groups of different materials
					for( auto mit = it->second.Batches.begin(); mit != it->second.Batches.end(); mit++ )
					{
						if( mit->second.Material == nullptr ) { continue; }

						// Upload the material to the shaders
						if( !vertexShader->UploadBatchMaterial( *mit->second.Material ) ||
							!pixelShader->UploadBatchMaterial( *mit->second.Material ) ||
							( geometryShader && !geometryShader->UploadBatchMaterial( *mit->second.Material ) ) )
						{
							continue;
						}

						// Now we need to render the instances in groups of 512
						uint32 instanceCount = (uint32)mit->second.InstanceTransforms.size();
						uint32 groupCount = ( ( instanceCount - 1 ) / RENDERER_MAX_INSTANCES_PER_BATCH ) + 1;

						for( uint32 i = 0; i < groupCount; i++ )
						{
							uint32 groupBegin = i * RENDERER_MAX_INSTANCES_PER_BATCH;
							uint32 groupEnd = Math::Min( groupBegin + RENDERER_MAX_INSTANCES_PER_BATCH, (uint32) instanceCount );
							uint32 groupSize = groupEnd - groupBegin;

							// Now we know how many instances are in this group, and the range in the list to look for the transforms
							// So, we need to upload a transform list to the shaders if they need it
							std::vector< Matrix > matrixList;
							for( uint32 j = groupBegin; j < groupEnd; j++ )
							{
								matrixList.push_back( mit->second.InstanceTransforms[ j ] );
							}

							if( !vertexShader->UploadBatchTransforms( matrixList ) ||
								!pixelShader->UploadBatchTransforms( matrixList ) ||
								( geometryShader && !geometryShader->UploadBatchTransforms( matrixList ) ) )
							{
								continue;
							}

							batchCount += groupSize;

							// Render the batch
							m_API->RenderBatch( groupSize, it->second.IndexBuffer->GetCount() );
						}
					}
				}
			}

		}
		else
		{
			// We only need to render the screen quad, so there is no 'Primitive Parameters' upload
			m_API->RenderScreenQuad();
			batchCount = 1;
		}

		//auto primEnd = std::chrono::high_resolution_clock::now();
		//std::chrono::duration< double, std::nano > primTime = primEnd - zbEnd;

		// Set everything back to the original state
		m_API->DisableAlphaBlending();
		m_API->EnableZBuffer();
		m_API->DetachRenderTarget();

		//auto passEnd = std::chrono::high_resolution_clock::now();
		//std::chrono::duration< double, std::nano > detachTime = primEnd - passEnd;

		//Console::WriteLine( "=====> Static Upload: ", uploadTime.count(), "ns \t GBuffer Upload: ", gbufferTime.count(), "ns \t Cluster Upload: ", clusterTime.count(), "ns \t ",
		//					"Lighting Upload: ", lightingTime.count(), "ns \t Target Setting: ", targetTime.count(), "ns \t Blending/Depth Settings: ", zbTime.count(), "ns \t ",
		//					"Main Primitive Pass: ", primTime.count(), "ns \t Detach: ", detachTime.count(), "ns" );

		return batchCount;
	}


	void Renderer::CollectBatches( BatchCollector& outBatches )
	{
		outBatches.Clear();

		const uint32 iterMax = (uint32)m_Scene->m_Primitives.size();
		for( uint32 i = 0; i < iterMax; i++ )
		{
			auto prim_ptr = m_Scene->m_Primitives[ i ];
			if( prim_ptr )
			{
				auto& prim_ref = *prim_ptr;

				// Check if we have to update this primitives world matrix
				if( prim_ref.m_bMatrixDirty )
				{
					m_API->CalculateWorldMatrix( prim_ref.m_Transform, prim_ref.m_WorldMatrix );
					m_API->TransformAABB( prim_ref.m_Transform, prim_ref.GetAABB(), prim_ref.m_OrientedBounds );
					prim_ref.m_bMatrixDirty = false;
				}

				if( prim_ref.m_bCacheDirty )
				{
					prim_ref.CacheMeshes();

				}

				if( m_API->CheckViewCull( prim_ref ) )
				{
					// Check if we need to update the world matrix
					prim_ref.CollectBatches( outBatches );
				}
			}
		}
		
	}

	bool Renderer::ApplyPostProcessEffect( const std::shared_ptr<PostProcessFX>& inFX, uint32 inFlags )
	{
		HYPERION_VERIFY( m_Scene, "[Renderer] Scene was null!" );

		if( !inFX || !inFX->IsValid() )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to apply post-process effect, the effect was null/invalid" );
			return false;
		}

		// We need to get the correct input texture, and the correct output texture
		std::shared_ptr< RRenderTarget > target		= nullptr;
		std::shared_ptr< RTexture2D > source		= nullptr;

		switch( inFX->GetRenderTarget() )
		{
		case PostProcessRenderTarget::BackBuffer:
			// If were targetting the back buffer, thats going to be the target
			target = m_API->GetRenderTarget();
			source = m_bPostProcessFrontTarget ? m_PostProcessBackBuffer : m_PostProcessFrontBuffer;
			break;
		case PostProcessRenderTarget::Intermediate:
			source = m_bPostProcessFrontTarget ? m_PostProcessFrontBuffer : m_PostProcessBackBuffer;
			target = m_bPostProcessFrontTarget ? m_PostProcessBackTarget : m_PostProcessFrontTarget;
			break;
		}

		if( !target || !target->IsValid() || !source || !source->IsValid() )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to apply post-process effect, render target/source is null/invalid" );
			return false;
		}

		// Now, we need to attach the shader and upload parameters
		auto shader = inFX->GetShader();
		
		if( !shader || !shader->Attach( source ) || !m_PostProcessVertexShader || !m_PostProcessVertexShader->Attach() )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to apply post-process effect, the shaders couldnt be attached" );
			return false;
		}

		if( !shader->UploadStaticParameters( *this, inFlags ) || !m_PostProcessVertexShader->UploadStaticParameters( *this, inFlags ) )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to apply post-process effect, the static parameters couldnt be uploaded" );
			return false;
		}

		// Before rendering, ensure the render target has been cleared
		m_API->ClearRenderTarget( target, Color4F( 0.f, 0.f, 0.f, 0.f ) );
		m_API->SetRenderTarget( target, nullptr );

		m_API->RenderScreenQuad();
		shader->Detach();
		m_PostProcessVertexShader->Detach();

		m_bPostProcessFrontTarget = !m_bPostProcessFrontTarget;

		return true;
	}


	bool Renderer::RebuildLightBuffer()
	{
		// Ensure the buffer is valid
		if( !m_LightBuffer || !m_LightBuffer->IsValid() )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to rebuild light buffer, the light buffer was null/invalid" );
			return false;
		}
		else if( !m_Scene )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to rebuild light buffer, the scene was null!" );
			return false;
		}

		std::vector< std::shared_ptr< ProxyLight > > lightList;
		for( auto it = m_Scene->LightsBegin(); it != m_Scene->LightsEnd(); it++ )
		{
			// We need to perform frustum culling, by using the radius of each light
			if( m_API->CheckViewCull( *( it->second ) ) )
			{
				lightList.push_back( it->second );
			}
		}

		if( !m_LightBuffer->UploadLights( lightList ) )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to rebuild light buffer, the lights couldnt be uploaded" );
			return false;
		}

		return true;
	}


	bool Renderer::RebuildViewClusters()
	{
		if( !m_ViewClusters || !m_ViewClusters->IsValid() )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to rebuild view clusters, view clusters were null/invalid" );
			return false;
		}

		// TODO: This shader needs to be split into two (RebuildClusters, ResetClusters)
		return DispatchComputeShader( m_BuildClustersShader, BUILD_CLUSTERS_MODE_REBUILD );
	}


	bool Renderer::AreViewClustersDirty() const
	{
		if( !m_ViewClusters ) { return false; }
		return m_ViewClusters->IsDirty();
	}


	bool Renderer::ResetViewClusters()
	{
		if( !m_ViewClusters || !m_ViewClusters->IsValid() )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to reset view clusters, the clusters were null/invalid" );
			return false;
		}
		
		// TODO: The 'build clusters' shader should just be split into two, and get rid of the whole notion of a 'mode'
		return DispatchComputeShader( m_BuildClustersShader, BUILD_CLUSTERS_MODE_CLEAR );
	}


	bool Renderer::DispatchComputeShader( const std::shared_ptr< RComputeShader >& inShader, uint32 inFlags )
	{
		if( !inShader || !inShader->IsValid() )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to dispatch compute shader, it was null or invalid" );
			return false;
		}

		if( !inShader->Attach() ) { return false; }

		// Upload parameters
		if( !inShader->UploadStaticParameters( *this, inFlags ) )						{ return false; }
		if( !inShader->UploadLightBuffer( *m_LightBuffer ) )							{ return false; }
		if( !inShader->UploadViewClusters( *m_ViewClusters ) )							{ return false; }
		if( inShader->RequiresGBuffer() && !inShader->UploadGBuffer( *m_GBuffer ) )		{ return false; }

		if( !inShader->Dispatch() ) { return false; }
		inShader->Detach();

		return true;
	}


	void Renderer::SetGBuffer( const std::shared_ptr< GBuffer >& inBuffer )
	{
		// Shutdown the current G-Buffer if there is one
		if( m_GBuffer )
		{
			m_GBuffer->Shutdown();
			m_GBuffer.reset();
		}

		// Assign new GBuffer
		m_GBuffer = inBuffer;
	}

}