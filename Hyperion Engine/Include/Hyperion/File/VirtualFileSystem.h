/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/File/VirtualFileSystem.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


#include "Hyperion/Hyperion.h"
#include "Hyperion/File/IFile.h"
#include "Hyperion/File/IDirectory.h"



namespace Hyperion
{

	/*
		Notes:
		* Each file bundle has a pool of file handles, that rotate along with the caller opening up files from within the bundle
		* Each bundle starts with a single handle, but if that handle is in use when we try and open a file from within, we just open a new
		  handle, and return a stream. We have to keep track of which handle is in use, and how long its been since it was in use
		* If a handle hasnt been used in a while, we automatically close it out, and continue

		* All of this means, we are going to need a thread to manage the handles, we can process the open commands on the thread where the call originated
	*/

	struct VirtualFileEntry
	{
		uint64 m_Offset;
		uint64 m_Length;
		uint32 m_Bundle;

		VirtualFileEntry()
			: m_Offset( 0 ), m_Length( 0 ), m_Bundle( 0 )
		{}

		VirtualFileEntry( const VirtualFileEntry& Other )
			: m_Offset( Other.m_Offset ), m_Length( Other.m_Length ), m_Bundle( Other.m_Bundle )
		{}
	};


	struct VirtualBundleHandle
	{
		std::unique_ptr< PhysicalFile > m_Handle;
		std::atomic< bool > m_InUse;
		std::chrono::time_point< std::chrono::steady_clock > m_LastUse;
	};


	struct VirtualBundle
	{
		FilePath m_Path;
		std::vector< std::shared_ptr< VirtualBundleHandle > > m_Handles;
		std::mutex m_HandleMutex;

		std::shared_ptr< VirtualBundleHandle > AquireHandle();
	};


	struct VirtualDirectoryEntry
	{
		std::vector< String > m_Files;
		std::vector< String > m_Directories;
	};


	class VirtualFile : public IFile
	{

	private:

		std::shared_ptr< VirtualBundleHandle > m_Handle;
		VirtualFileEntry m_Entry;

		VirtualFile( const FilePath& inPath, const std::shared_ptr< VirtualBundleHandle >& inHandle, const VirtualFileEntry& inEntry );

	public:

		VirtualFile() = delete;
		VirtualFile( const VirtualFile& ) = delete;
		VirtualFile( VirtualFile&& ) = delete;

		VirtualFile& operator=( const VirtualFile& ) = delete;
		VirtualFile& operator=( VirtualFile&& ) = delete;

		~VirtualFile();

		bool IsValid() const final;
		size_t GetSize() const final;

		std::basic_streambuf< byte >* GetStreamBuffer() final;

		bool CanReadStream() const final;
		bool CanWriteStream() const final;

		bool IsClamped() const final;
		size_t GetClampedRangeBegin() const final;
		size_t GetClampedRangeEnd() const final;

		friend class VirtualFileSystem;
	};


	class VirtualDirectory : public IDirectory
	{

	protected:

		std::shared_ptr< VirtualDirectoryEntry > m_Entry;

		VirtualDirectory( const FilePath& inPath, const std::shared_ptr< VirtualDirectoryEntry >& inEntry );

		void CacheContents() final;

	public:

		VirtualDirectory() = delete;

		friend class VirtualFileSystem;
	};


	class VirtualFileSystem
	{

	private:
		// We want a single lookup table for all files
		// This means we need some type of........
		static std::map< String, VirtualFileEntry > m_FileManifest;
		static std::map< String, std::shared_ptr< VirtualDirectoryEntry > > m_DirectoryManifest;
		static std::map< uint32, VirtualBundle > m_BundleManifest;

		static bool m_bInitialized;
		static uint32 m_LastBundleIdentifier;

	public:

		static bool Initialize( bool bDiscoverAssets );
		static void Shutdown();

		static std::unique_ptr< VirtualFile > OpenFile( const FilePath& inPath );
		static bool FileExists( const FilePath& inPath );

		static std::unique_ptr< VirtualDirectory > OpenDirectory( const FilePath& inPath );
		static bool DirectoryExists( const FilePath& inPath );

	};


}