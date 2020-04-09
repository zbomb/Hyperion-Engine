/*==================================================================================================
	Hyperion Engine
	Source/File/UnifiedFileSystem.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/File/UnifiedFileSystem.h"
#include "Hyperion/File/VirtualFileSystem.h"
#include "Hyperion/File/PhysicalFileSystem.h"
#include "Hyperion/File/NetworkFileSystem.h"



namespace Hyperion
{

	ConsoleVar< String > g_CVar_ContentSystem(
		"fs_content_system", "The filesystem to use when loading/searching content, Valid Inputs: [v|d|n] (virtual, disk or network)", "v"
	);

	bool UnifiedFileSystem::m_bInit( false );
	FileSystem UnifiedFileSystem::m_ContentSystem( FileSystem::Disk );


	void UnifiedFileSystem::Initialize()
	{
		// Guard against double init
		if( m_bInit )
		{
			Console::WriteLine( "[ERROR] FileSystem: Attempt to initialize twice!" );
			return;
		}

		m_bInit = true;
		auto targetSys = g_CVar_ContentSystem.GetValue().ToLower();
		String systemDescription;

		if( targetSys == "v" )
		{
			m_ContentSystem		= FileSystem::Virtual;
			systemDescription	= "virtual";
		}
		else if( targetSys == "n" )
		{
			m_ContentSystem		= FileSystem::Network;
			systemDescription	= "network";
		}
		else
		{
			if( targetSys != "d" )
			{
				Console::WriteLine( "[WARNING] FileSystem: The selected content system is invalid (", targetSys, ").. defaulting to disk" );
			}

			m_ContentSystem		= FileSystem::Disk;
			systemDescription	= "disk";
		}

		Console::WriteLine( "[Status] FileSystem: Initializing.. selected content system '", systemDescription, "'" );

		// First, attempt to initialize the virtual file system
		if( !VirtualFileSystem::Initialize( m_ContentSystem == FileSystem::Virtual ) )
		{
			if( m_ContentSystem == FileSystem::Virtual )
			{
				Console::WriteLine( "[ERROR] FileSystem: Failed to initialize virtual file system.. This was the selected content source, falling back to 'disk'!" );
				m_ContentSystem = FileSystem::Disk;
			}
			else
			{
				Console::WriteLine( "[ERROR] FileSystem: Failed to initialize virtual file system!" );
			}
		}

		// TODO: Attempt to initialize the network file system

		// Finally, initialize the physical file system
		if( !PhysicalFileSystem::Initialize( m_ContentSystem == FileSystem::Disk ) )
		{
			if( m_ContentSystem == FileSystem::Disk )
			{
				Console::WriteLine( "[ERROR] FileSystem: Failed to initialize physical file system.. This was the selected content source, NO content will be available!!" );
			}
			else
			{
				Console::WriteLine( "[ERROR] FileSystem: Failed to initialize physical file system" );
			}
		}

	}


	void UnifiedFileSystem::Shutdown()
	{
		Console::WriteLine( "[Status] FileSystem: Shutting down..." );
		VirtualFileSystem::Shutdown();
		PhysicalFileSystem::Shutdown();
	}


	FileSystem UnifiedFileSystem::DetermineFileSystem( const FilePath& inPath )
	{
		// We need to determine which file system to use for this file path
		FileSystem Output = inPath.GetSystem();

		if( Output == FileSystem::None )
		{
			if( inPath.GetLocal() == LocalPath::Content )
			{
				return m_ContentSystem;
			}
			else
			{
				return FileSystem::Disk;
			}
		}

		return Output;
	}


	std::unique_ptr< IFile > UnifiedFileSystem::OpenFile( const FilePath& inPath )
	{
		FileSystem targetSystem = DetermineFileSystem( inPath );

		switch( targetSystem )
		{
		case FileSystem::Network:
			HYPERION_NOT_IMPLEMENTED( "Accessing network file system through unified file system" );
			return nullptr;
		case FileSystem::Virtual:
			return VirtualFileSystem::OpenFile( inPath );
		case FileSystem::Disk:
		default:
			return PhysicalFileSystem::OpenFile( inPath, FileMode::Read );
		}
	}


	bool UnifiedFileSystem::FileExists( const FilePath& inPath )
	{
		FileSystem targetSystem = DetermineFileSystem( inPath );

		switch( targetSystem )
		{
		case FileSystem::Network:
			HYPERION_NOT_IMPLEMENTED( "Accessing network file system through unified file system" );
			return false;
		case FileSystem::Virtual:
			return VirtualFileSystem::FileExists( inPath );
		case FileSystem::Disk:
		default:
			return PhysicalFileSystem::FileExists( inPath );
		}
	}


	std::unique_ptr< IDirectory > UnifiedFileSystem::OpenDirectory( const FilePath& inPath )
	{
		FileSystem targetSystem = DetermineFileSystem( inPath );

		switch( targetSystem )
		{
		case FileSystem::Network:
			HYPERION_NOT_IMPLEMENTED( "Accessing network file system thorugh unified file system" );
			return nullptr;
		case FileSystem::Virtual:
			return VirtualFileSystem::OpenDirectory( inPath );
		case FileSystem::Disk:
		default:
			return PhysicalFileSystem::OpenDirectory( inPath );
		}
	}


	bool UnifiedFileSystem::DirectoryExists( const FilePath& inPath )
	{
		FileSystem targetSystem = DetermineFileSystem( inPath );

		switch( targetSystem )
		{
		case FileSystem::Network:
			HYPERION_NOT_IMPLEMENTED( "Accessing network file system through unified file system" );
			return false;
		case FileSystem::Virtual:
			return VirtualFileSystem::DirectoryExists( inPath );
		case FileSystem::Disk:
		default:
			return PhysicalFileSystem::DirectoryExists( inPath );
		}
	}


}