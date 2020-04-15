/*==================================================================================================
	Hyperion Engine
	Source/Renderer/Renderer.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Renderer/Proxy/ProxyScene.h"
#include "Hyperion/Renderer/Proxy/ProxyPrimitive.h"
#include "Hyperion/Renderer/Proxy/ProxyLight.h"
#include "Hyperion/Core/RenderManager.h"
#include "Hyperion/Core/ThreadManager.h"
#include "Hyperion/Core/Asset.h"
#include "Hyperion/Assets/TextureAsset.h"
#include "Hyperion/Library/Math.h"

/*
	Include various graphics APIs?
*/
#if HYPERION_OS_WIN32
#include "Hyperion/Renderer/DirectX11/DirectX11Graphics.h"
#endif


namespace Hyperion
{


	Renderer::Renderer( std::shared_ptr< IGraphics >& inAPI, const IRenderOutput& inOutput, const ScreenResolution& inRes, bool bVSync )
		: m_API( inAPI ), m_Output( inOutput ), m_Resolution( inRes ), m_bVSync( bVSync ), m_Scene( std::make_shared< ProxyScene >() )
	{
		HYPERION_VERIFY( m_API != nullptr, "Failed to load graphics api!" );
	}


	Renderer::~Renderer()
	{
		Shutdown();
	}


	bool Renderer::Initialize()
	{
		if( !m_API )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to initialize.. graphics api was null!" );
			return false;
		}

		// We need to setup and init our graphics api
		m_API->SetResolution( m_Resolution );
		m_API->SetVSync( m_bVSync );

		if( !m_API->Initialize( m_Output ) )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to initialize graphics API!" );
			std::terminate();
			return false;
		}

		// Initialize the scene
		m_Scene->Initialize();
		return true;
	}


	void Renderer::Shutdown()
	{
		// Shutdown the scene
		if( m_Scene )
		{
			m_Scene.reset();
		}

		// We need to shutdown the graphics API
		if( m_API )
		{
			m_API.reset();
		}
	}


	void Renderer::Frame()
	{
		// First, we need to update the proxy scene
		UpdateScene();

		// Next, we need to prepare the API for the frame
		m_API->BeginFrame();

		// TODO: Render scene

		m_API->EndFrame();
	}


	void Renderer::UpdateScene()
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
		auto nextCommand = m_Commands.PopValue();
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
				break;
			}

			// Pop the next command in the list
			nextCommand = m_Commands.PopValue();
		}
	}


	void Renderer::AddImmediateCommand( std::unique_ptr< RenderCommandBase >&& inCommand )
	{
		m_ImmediateCommands.Push( std::move( inCommand ) );
	}

	void Renderer::AddCommand( std::unique_ptr< RenderCommandBase >&& inCommand )
	{
		m_Commands.Push( std::move( inCommand ) );
	}


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


	bool Renderer::IncreaseTextureAssetLOD( std::shared_ptr< TextureAsset >& inAsset, uint8 inMaxLevel, const std::vector< std::vector< byte > >& inData )
	{
		HYPERION_VERIFY( m_API, "Failed to update texture asset, API was null!" );

		if( !inAsset )
		{
			Console::WriteLine( "[ERROR] Failed to update texture asset, the provided asset reference was invalid!" );
			return false;
		}

		uint32 textureIdentifier	= inAsset->GetIdentifier();
		auto textureHeader			= inAsset->GetHeader();
		auto texturePath			= inAsset->GetPath();

		// Ensure 'inMaxLevel' is a valid value
		if( textureHeader.LODs.size() <= inMaxLevel )
		{
			Console::WriteLine( "[ERROR] Renderer: Attempt to load invalid LOD level for texture \"", texturePath, "\" at LOD = ", inMaxLevel, " where the number of levels is only ", textureHeader.LODs.size() );
			return false;
		}

		// Calculate how many LODs were going to be loading
		uint8 lodCount = (uint8) ( textureHeader.LODs.size() - inMaxLevel );

		// Ensure we have the correct number of data blocks
		if( inData.size() != lodCount )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to add LODs to texture, invalid data!" );
			return false;
		}

		auto& maxLOD = textureHeader.LODs.at( inMaxLevel );

		// Now, we need to create a new texture, and supply it with this data
		Texture2DParameters params;

		params.CanCPURead	= false;
		params.Dynamic		= false;
		params.Format		= textureHeader.Format;
		params.Width		= maxLOD.Width;
		params.Height		= maxLOD.Height;
		params.MipLevels	= lodCount;
		params.Target		= TextureBindTarget::Shader;

		// Now we need to build the data for the D3D11 subresource structure
		for( uint8 i = 0; i < lodCount; i++ )
		{
			uint8 thisLevel = i + inMaxLevel;
			auto& lodInfo = textureHeader.LODs.at( thisLevel );

			Texture2DMipData mip;

			mip.Data			= inData.at( i ).data();
			mip.RowDataSize		= lodInfo.RowSize;

			params.Data.push_back( mip );
		}

		// Now, if we are able to create textures async with the current API, we will just do that, and add a command
		// to swap this texture out with the active one, otherwise, the texture will heave to  be created on the main render thread
		if( m_API->AllowAsyncTextureCreation() )
		{
			auto newTex = m_API->CreateTexture2D( params );
			if( !newTex )
			{
				Console::WriteLine( "[ERROR] Renderer: Failed to add LODs to texture, API call to create the new texture failed!" );
				return false;
			}

			AddImmediateCommand(
				std::make_unique< RenderCommand >(
					[ this, textureIdentifier, newTex ] ( Renderer& r )
					{
						// How swap out the textures
						auto entry = m_TextureCache.find( textureIdentifier );
						if( entry == m_TextureCache.end() )
						{
							m_TextureCache.emplace( textureIdentifier, newTex );
						}
						else
						{
							if( entry->second )
							{
								// Were not going to just assign the pointer
								// We are going to swap the contained API ref inside the pointer, so any existing
								// texture pointers dont break after this operation is complete
								entry->second->Swap( *newTex );

								if( newTex->IsValid() )
								{
									newTex->Shutdown();
								}
							}
							else
							{
								entry->second = newTex;
							}
						}

					} )
			);
		}
		else
		{
			// Now, everything we do has to be on the render thread, so we have to pass the data through to create the texture
			AddImmediateCommand(
				std::make_unique< RenderCommand >(
					[ this, textureIdentifier, params ] ( Renderer& r )
					{
						// First, create the texture
						auto newTex = m_API->CreateTexture2D( params );
						if( !newTex )
						{
							Console::WriteLine( "[ERROR] Renderer: Failed to add LODs to texture asset, API call to create the new texture failed (on render thread)!" );
						}
						else
						{
							// Hotswap the textures
							auto entry = m_TextureCache.find( textureIdentifier );
							if( entry == m_TextureCache.end() )
							{
								m_TextureCache.emplace( textureIdentifier, newTex );
							}
							else
							{
								// We want to keep the same pointer in the cache, so existing references dont break
								// So, were going to swap the underlying API object inside the pointers instead
								if( entry->second )
								{
									entry->second->Swap( *newTex );

									if( newTex->IsValid() )
									{
										newTex->Shutdown();
									}
								}
								else
								{
									entry->second = newTex;
								}
							}
						}

					} )
			);
		}

		return true;
	}

	
	bool Renderer::LowerTextureAssetLOD( std::shared_ptr< TextureAsset >& inAsset, uint8 inMaxLevel )
	{
		HYPERION_VERIFY( m_API, "Failed to update texture asset, API was null!" );

		if( !inAsset )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to lower texture asset LOD, the asset supplied was invalid" );
			return false;
		}

		auto textureIdentifier	= inAsset->GetIdentifier();
		auto textureHeader		= inAsset->GetHeader();
		auto texturePath		= inAsset->GetPath();

		if( textureHeader.LODs.size() <= inMaxLevel )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to lower texture asset LOD, the target LOD level was invalid!" );
			return false;
		}

		uint8 lodCount	= (uint8) ( textureHeader.LODs.size() - inMaxLevel );
		auto& maxLOD	= textureHeader.LODs.at( inMaxLevel );

		// Setup parameters to create the new texture
		Texture2DParameters params;

		params.CanCPURead	= false;
		params.Dynamic		= false;
		params.Format		= textureHeader.Format;
		params.Width		= maxLOD.Width;
		params.Height		= maxLOD.Height;
		params.MipLevels	= lodCount;
		params.Target		= TextureBindTarget::Shader;

		// Were not going to provide initial data, instead, were going to copy the data from the existing texture
		// But we still need to check if we can create the texture 'off-thread' or not
		if( m_API->AllowAsyncTextureCreation() )
		{
			auto newTex = m_API->CreateTexture2D( params );
			if( !newTex )
			{
				Console::WriteLine( "[ERROR] Renderer: Failed to lower texture LOD, API call to create the texture failed!" );
				return false;
			}

			// Now, we need a render command to copy the existing texture into our new one
			AddImmediateCommand(
				std::make_unique< RenderCommand >(
					[ this, textureIdentifier, inMaxLevel, textureHeader, lodCount, newTex ] ( Renderer& r ) mutable
					{
						auto entry = m_TextureCache.find( textureIdentifier );
						if( entry == m_TextureCache.end() || !entry->second || !entry->second->IsValid() )
						{
							// How can we feed this back into the algorithm?
							// Or recover from this error? i.e. Tell the AA thread this texture is misssing?
							Console::WriteLine( "[ERROR] Renderer: Attempt to lower texture LOD, but the target texture is not loaded/valid on the render thread!" );
						}
						else
						{
							// Now, we need to copy each level into the new texture
							auto oldTex = entry->second;
							auto oldLodCount = oldTex->GetMipLevels();
							bool bFailed = false;

							for( uint8 i = inMaxLevel; i < textureHeader.LODs.size(); i++ )
							{
								auto& lodInfo = textureHeader.LODs.at( i );
								auto newTexIndex = lodCount - ( textureHeader.LODs.size() - i );
								auto oldTexIndex = oldLodCount - ( textureHeader.LODs.size() - i );

								if( !m_API->CopyLODTexture2D( oldTex, newTex,
															  0, 0,
															  lodInfo.Width, lodInfo.Height,
															  0, 0,
															  (uint8)oldTexIndex, (uint8)newTexIndex ) )
								{
									Console::WriteLine( "[ERROR] Renderer: Attempt to lower texture LOD, but one of the mip levels couldnt be copied!" );
									bFailed = true;
									break;
								}
							}

							if( !bFailed )
							{
								// Texture hotswap
								entry->second->Swap( *newTex );
								newTex->Shutdown();
								newTex.reset();
							}
							else
							{
								// TODO: Feedback into AA system
							}
						}
					} )
			);
		}
		else
		{
			AddImmediateCommand(
				std::make_unique< RenderCommand >(
					[ this, textureIdentifier, params, inMaxLevel, textureHeader, lodCount ] ( Renderer& r ) mutable
					{
						// First, lets check if the existing texture is valid
						auto entry = m_TextureCache.find( textureIdentifier );
						if( entry == m_TextureCache.end() || !entry->second || !entry->second->IsValid() )
						{
							Console::WriteLine( "[ERROR] Renderer: Attempt to lower texture LOD.. but the target texture was invalid/null!" );
							// TODO: Feedback to the AA system?
						}
						else
						{
							// Next, create the new texture
							auto newTex = m_API->CreateTexture2D( params );
							if( !newTex )
							{
								Console::WriteLine( "[ERROR] Renderer: Attempt to lower texture LOD.. but couldnt create new texture (on render thread)!" );
								// TODO: Feedback to AA system?
							}
							else
							{
								auto oldTex			= entry->second;
								auto oldLODCount	= oldTex->GetMipLevels();
								bool bFailed		= false;

								for( uint8 i = inMaxLevel; i < textureHeader.LODs.size(); i++ )
								{
									auto& lodInfo		= textureHeader.LODs.at( i );
									auto newTexIndex	= lodCount - ( textureHeader.LODs.size() - i );
									auto oldTexIndex	= oldLODCount - ( textureHeader.LODs.size() - i );

									if( !m_API->CopyLODTexture2D( oldTex, newTex,
																  0, 0,
																  lodInfo.Width, lodInfo.Height,
																  0, 0,
																  (uint8)oldTexIndex, (uint8)newTexIndex ) )
									{
										Console::WriteLine( "[ERROR] Renderer: Attempt to lower texture LOD.. but the API copy call failed when copying to new texture!" );
										bFailed = true;
										break;
									}
								}

								if( !bFailed )
								{
									// Texture hotswap
									entry->second->Swap( *newTex );
									newTex->Shutdown();
									newTex.reset();
								}
								else
								{
									// TODO: Feedback into AA system
								}
							}
						}
					} )
			);
		}

		return true;
	}

	
	void Renderer::RemoveTextureAsset( uint32 inIdentifier )
	{
		AddImmediateCommand(
			std::make_unique< RenderCommand >(
				[ this, inIdentifier ] ( Renderer& r )
				{
					// First, lets try and find the entry
					auto entry = m_TextureCache.find( inIdentifier );
					if( entry != m_TextureCache.end() )
					{
						m_TextureCache.erase( entry );
					}

				} )
		);
	}


	void Renderer::ClearTextureAssetCache()
	{
		AddImmediateCommand(
			std::make_unique< RenderCommand >(
				[ this ] ( Renderer& r )
				{
					m_TextureCache.clear();
				} )
		);
	}

}