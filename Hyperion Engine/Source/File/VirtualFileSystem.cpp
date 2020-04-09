/*==================================================================================================
	Hyperion Engine
	Source/File/VirtualFileSystem.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/File/VirtualFileSystem.h"
#include "Hyperion/File/PhysicalFileSystem.h"
#include "Hyperion/Tools/HVBReader.h"
#include "Hyperion/Core/AssetManager.h"


constexpr auto HYPERION_VFS_MAX_HANDLE_IDLE_TIME_MINS = 5;


namespace Hyperion
{

	/*
		Static Declarations
	*/
	std::map< String, VirtualFileEntry > VirtualFileSystem::m_FileManifest;
	std::map< String, std::shared_ptr< VirtualDirectoryEntry > > VirtualFileSystem::m_DirectoryManifest;
	std::map< uint32, VirtualBundle > VirtualFileSystem::m_BundleManifest;
	bool VirtualFileSystem::m_bInitialized( false );
	uint32 VirtualFileSystem::m_LastBundleIdentifier( 0 );


	/*--------------------------------------------------------------------------------------------
		VirtualFile (IFile)
	--------------------------------------------------------------------------------------------*/
	VirtualFile::VirtualFile( const FilePath& inPath, const std::shared_ptr< VirtualBundleHandle >& inHandle, const VirtualFileEntry& inEntry )
		: IFile( inPath ), m_Handle( inHandle ), m_Entry( inEntry )
	{
		// Validate the parameters
		if( !inHandle || inEntry.m_Bundle == 0 )
		{
			Console::WriteLine( "[ERROR] VirtualFileSystem: Attempt to open virtual file.. but the handle/bundle was invalid!" );

			m_Handle.reset();
			m_Entry.m_Bundle = 0;
		}
	}


	VirtualFile::~VirtualFile()
	{
		if( m_Handle && m_Handle->m_InUse )
		{
			// We need to mark the contained handle as released
			m_Handle->m_LastUse		= std::chrono::steady_clock::now();
			m_Handle->m_InUse		= false;

			m_Handle.reset();
		}
	}


	bool VirtualFile::IsValid() const
	{
		return m_Handle && m_Handle->m_Handle && m_Entry.m_Bundle != 0;
	}


	size_t VirtualFile::GetSize() const
	{
		return m_Entry.m_Length;
	}


	std::basic_streambuf< byte >* VirtualFile::GetStreamBuffer()
	{
		return IsValid() ? m_Handle->m_Handle->GetStreamBuffer() : nullptr;
	}


	bool VirtualFile::CanReadStream() const
	{
		return true;
	}


	bool VirtualFile::CanWriteStream() const
	{
		return false;
	}


	bool VirtualFile::IsClamped() const
	{
		return true;
	}


	size_t VirtualFile::GetClampedRangeBegin() const
	{
		return m_Entry.m_Offset;
	}


	size_t VirtualFile::GetClampedRangeEnd() const
	{
		return m_Entry.m_Offset + m_Entry.m_Length;
	}

	/*--------------------------------------------------------------------------------------------
		VirtualDirectory (IDirectory)
	--------------------------------------------------------------------------------------------*/
	VirtualDirectory::VirtualDirectory( const FilePath& inPath, const std::shared_ptr< VirtualDirectoryEntry >& inEntry )
		: IDirectory( inPath, inEntry != nullptr ), m_Entry( inEntry )
	{
	}


	void VirtualDirectory::CacheContents()
	{
		// Clear out the currently cached files and directories
		{
			std::vector< FilePath >().swap( m_Files );
			std::vector< FilePath >().swap( m_Directories );
		}

		// Fill out the 'IDirectory' members from the entry pointer we have stored
		if( m_Entry )
		{
			for( auto dir : m_Entry->m_Directories )
			{
				m_Directories.push_back(
					FilePath(
						dir, LocalPath::Game, FileSystem::Virtual // TODO: Are the directories in the iterator full paths? Or are they just the name of the folder?
					)
				);
			}

			for( auto f : m_Entry->m_Files )
			{
				// FilePath( m_Path / f, LocalPath::Game, FileSystem::Virtual ); ?

				m_Files.push_back(
					FilePath(
						f, LocalPath::Game, FileSystem::Virtual // TODO: Are the directories in the iterator full paths? Or are they just the name of the file?
					)
				);
			}
		}
	}

	/*--------------------------------------------------------------------------------------------
		VirtualBundle
	--------------------------------------------------------------------------------------------*/
	std::shared_ptr< VirtualBundleHandle > VirtualBundle::AquireHandle()
	{
		// We need to see if there is an available handle not in use, this requires the use of a mutex
		{
			std::lock_guard< std::mutex > lock( m_HandleMutex );
			std::shared_ptr< VirtualBundleHandle > targetHandle;

			for( auto It = m_Handles.begin(); It != m_Handles.end(); )
			{
				auto ptr = *It;

				if( !ptr )
				{
					It = m_Handles.erase( It );
					continue;
				}

				if( !targetHandle )
				{
					if( !ptr->m_InUse )
					{
						targetHandle = ptr;
					}
				}

				if( !ptr->m_InUse )
				{
					if( !targetHandle ) 
					{ 
						targetHandle = ptr; 
					}
					else
					{
						// Check if this handle is old enough to erase
						auto dur = std::chrono::duration_cast<std::chrono::minutes>( std::chrono::steady_clock::now() - ptr->m_LastUse ).count();
						if( dur > HYPERION_VFS_MAX_HANDLE_IDLE_TIME_MINS )
						{
							It = m_Handles.erase( It );
							continue;
						}
					}
				}
				
				It++;
			}

			// If we dont have a handle, we need to open a new one
			if( !targetHandle )
			{
				targetHandle			= std::make_shared< VirtualBundleHandle >();
				targetHandle->m_Handle	= PhysicalFileSystem::OpenFile( m_Path, FileMode::Read );

				// Ensure the file was able to be opened
				if( !targetHandle->m_Handle || !targetHandle->m_Handle->IsValid() )
				{
					Console::WriteLine( "[ERROR] VirtualFileSystem: Failed to create a new handle to bundle '", m_Path.ToString( true ), "!" );
					targetHandle.reset();

					return nullptr;
				}
				else
				{
					// Insert the new handle into the list
					m_Handles.push_back( targetHandle );
				}
			}

			// At this point, targetHandle is valid!
			targetHandle->m_InUse		= true;
			targetHandle->m_LastUse		= std::chrono::steady_clock::now();

			return targetHandle;
		}
	}


	/*--------------------------------------------------------------------------------------------
		VirtualFileSystem
	--------------------------------------------------------------------------------------------*/
	bool VirtualFileSystem::Initialize( bool bDiscoverAssets )
	{
		// Protect against double initialization
		if( m_bInitialized )
		{
			Console::WriteLine( "[ERROR] VirtualFileSystem: Failed to initialize! Already initialized once?" );
			return false;
		}

		m_bInitialized = true;

		// Get a list of all bundle files in the bundle directory
		std::vector< FilePath > bundleList;
		PhysicalFileSystem::FindFiles(
			FilePath( String( "bundles/" ), LocalPath::Game, FileSystem::Disk ),
			bundleList,
			[] ( const FilePath& In )
			{
				return In.Extension().ToLower().Equals( ".hvb" );
			},
			true
		);

		// Loop through all bundle files found, and process each indivisually
		for( auto& path : bundleList )
		{
			auto f = PhysicalFileSystem::OpenFile( path, FileMode::Read );
			if( !f || !f->IsValid() )
			{
				Console::WriteLine( "[WARNING] VirtualFileSystem: Failed to open bundle '", path, "', so the files contained in this bundle will be unavailable at runtime" );
				continue;
			}

			HVBReader bundleReader( *f );
			HVBReader::HeaderData bundleHeader;
			auto headerReadResult = bundleReader.ReadHeader( bundleHeader );

			if( headerReadResult != HVBReader::ReadHeaderResult::Success )
			{
				Console::WriteLine( "[WARNING] VirtualFileSystem: Failed to process bundle '", path, "' (Header Read Error #", (uint32)headerReadResult,
									") so the files contained in this bundle will be unavailable at runtime" );
				continue;
			}

			if( !bundleReader.StartFiles() )
			{
				Console::WriteLine( "[WARNING] VirtualFileSystem: Failed to process bundle '", path, "' (Start File Error) so the files contained in this bundle will be unavailable at runtime" );
				continue;
			}

			// Create the virtual bundle structure to hold this data
			auto bundleIdentifier	= ++m_LastBundleIdentifier;
			auto& newBundle			= m_BundleManifest[ bundleIdentifier ];
			newBundle.m_Path		= path;
			uint32 assetCounter		= 0;

			while( bundleReader.NextFile() )
			{
				HVBReader::FileInfo Info;
				auto fileReadResult = bundleReader.ReadFileInfo( Info );

				if( fileReadResult != HVBReader::ReadFileResult::Success )
				{
					Console::WriteLine( "[WARNING] VirtualFileSystem: Failed to process bundle '", path, "' (File Info Read Error #", (uint32) fileReadResult,
										") so the files contained in this bundle will be unavailable at runtime" );
					continue;
				}

				// Check for file collisions
				auto& fileEntry = m_FileManifest[ Info.Path ];
				if( fileEntry.m_Bundle != 0 )
				{
					Console::WriteLine( "[WARNING] VirtualFileSystem: Failed to process file '", Info.Path, "' because there was a file name collision!" );
					continue;
				}

				fileEntry.m_Bundle	= bundleIdentifier;
				fileEntry.m_Length	= Info.FileLength;
				fileEntry.m_Offset	= Info.FileOffset;

				// If were discovering assets through this file system, then register the assets with the manager
				if( bDiscoverAssets && Info.AssetIdentifier != 0 )
				{
					AssetManager::RegisterAsset( Info.AssetIdentifier, Info.Path );
					assetCounter++;
				}
			}

			if( bDiscoverAssets )
			{
				Console::WriteLine( "[Status] FileSystem: Discovered ", assetCounter, " assets on the virtual disk" );
			}

			// TODO: Add Directory Support
			// To do this, we need some way to find all directories in this bundle
			// We could either...
			// A) Break down each files path, into directories, and create one for each if not exists
			// B) Store a seperate list of all directories in the bundle as well
			//
			// A, would require more processing power, but saves on memory and disk space
			// B, would require less processing power, but costs more in memory and disk space
		}

		return true;
	}


	void VirtualFileSystem::Shutdown()
	{
		m_FileManifest.clear();
		m_BundleManifest.clear();
		m_DirectoryManifest.clear();
	}


	std::unique_ptr< VirtualFile > VirtualFileSystem::OpenFile( const FilePath& inPath )
	{
		// Were going to override the 'system' of the path to virtual
		auto newPath( inPath );
		newPath.SetSystem( FileSystem::Virtual );

		// Validate
		if( newPath.GetLocal() == LocalPath::Root || newPath.GetLocal() == LocalPath::Documents )
		{
			Console::WriteLine( "[ERROR] VirtualFileSystem: Attempt to open a 'root' or 'data' file.. you have to use the physical file system for this!" );
			return nullptr;
		}

		// Attempt to lookup the file info
		auto strPath = newPath.ToString();

		auto fileInfo = m_FileManifest.find( strPath );
		if( fileInfo == m_FileManifest.end() )
		{
			Console::WriteLine( "[WARNING] VirtualFileSystem: Attempt to open a non-existant file '", strPath, "'" );
			return nullptr;
		}

		// Now, find the bundle that contains this file
		auto bundleInfo = m_BundleManifest.find( fileInfo->second.m_Bundle );
		if( bundleInfo == m_BundleManifest.end() )
		{
			Console::WriteLine( "[ERROR] VirtualFileSystem: Bundle identifier for file '", strPath, "' was invalid (", fileInfo->second.m_Bundle, ")" );
			return nullptr;
		}

		// Next, we need to get an active handle
		auto fileHandle = bundleInfo->second.AquireHandle();
		if( !fileHandle )
		{
			Console::WriteLine( "[ERROR] VirtualFIleSystem: Failed to open file '", strPath, "' because a bundle handle couldnt be aquired for bundle #", fileInfo->second.m_Bundle, "!" );
			return nullptr;
		}

		// Now, we can build a virtual file now that we know the offset, length and have a handle
		return std::unique_ptr< VirtualFile >( new VirtualFile( newPath, fileHandle, fileInfo->second ) );
	}


	bool VirtualFileSystem::FileExists( const FilePath& inPath )
	{
		// Validate the path
		auto local = inPath.GetLocal();
		if( local == LocalPath::Root || local == LocalPath::Documents )
		{
			Console::WriteLine( "[ERROR] VirtualFileSystem: Attempt to check if a 'root' or 'data' file exists.. the virtual file system is not able to access these files!" );
			return false;
		}

		// Lookup in the manifest
		return m_FileManifest.find( inPath.ToString() ) != m_FileManifest.end();
	}


	std::unique_ptr< VirtualDirectory > VirtualFileSystem::OpenDirectory( const FilePath& inPath )
	{
		// Were going to override the 'system' of the path to virtual
		auto newPath( inPath );
		newPath.SetSystem( FileSystem::Virtual );

		// Validate
		if( newPath.GetLocal() == LocalPath::Root || newPath.GetLocal() == LocalPath::Documents )
		{
			Console::WriteLine( "[ERROR] VirtualFileSystem: Attempt to open a 'root' or 'data' directory.. you have to use the physical file system for this!" );
			return nullptr;
		}

		// Attempt to find the directory entry
		auto strPath	= newPath.ToString();
		auto dirEntry	= m_DirectoryManifest.find( strPath );

		if( dirEntry == m_DirectoryManifest.end() || !dirEntry->second )
		{
			Console::WriteLine( "[WARNING] VirtualFilesystem: Attempt to open a non-existant directory '", strPath, "'" );
			return nullptr;
		}

		// Return the directory pointer
		return std::unique_ptr< VirtualDirectory >( new VirtualDirectory( newPath, dirEntry->second ) );
	}


	bool VirtualFileSystem::DirectoryExists( const FilePath& inPath )
	{
		// Validate the path
		auto local = inPath.GetLocal();
		if( local == LocalPath::Root || local == LocalPath::Documents )
		{
			Console::WriteLine( "[ERROR] VirtualFileSystem: Attempt to check if a 'root' or 'data' direcotry exists.. you have to use the physical file system for this!" );
			return false;
		}

		// Attempt to find the entry
		auto strPath = inPath.ToString();
		auto dirEntry = m_DirectoryManifest.find( strPath );

		return dirEntry != m_DirectoryManifest.end() && dirEntry->second;
	}


}