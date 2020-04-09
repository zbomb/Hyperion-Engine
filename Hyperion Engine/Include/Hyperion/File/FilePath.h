/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/File/FilePath.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/String.h"

#include <filesystem>


namespace Hyperion
{

	enum class LocalPath
	{
		Root		= 0,
		Game		= 1,
		Documents	= 2,
		Content		= 3
	};

	enum class FileSystem
	{
		None		= 0,
		Disk		= 1,
		Virtual		= 2,
		Network		= 4
	};


	class FilePath
	{

	private:

		LocalPath m_Local;
		FileSystem m_System;
		std::filesystem::path m_Path;

		FilePath( const std::filesystem::path& inPath, LocalPath inLocal, FileSystem inSystem );
		void _Verify();

	public:

		FilePath();
		explicit FilePath( const String& inPath, LocalPath inLocal = LocalPath::Game, FileSystem inSystem = FileSystem::None );
		explicit FilePath( LocalPath inLocal, FileSystem inSystem = FileSystem::None );
		FilePath( const String& inPath, FileSystem inSystem );

		FilePath( const FilePath& Other );
		FilePath( FilePath&& Other ) noexcept;

		FilePath& operator=( const FilePath& Other );
		FilePath& operator=( FilePath&& Other ) noexcept;

		String ToString( bool bAbsolutePath = false ) const;
		std::filesystem::path ToCPath( bool bAbsolutePath = false ) const;

		FilePath ToRootPath() const;

		inline bool IsRootPath() const { return m_Local == LocalPath::Root; }
		inline LocalPath GetLocal() const { return m_Local; }
		inline FileSystem GetSystem() const { return m_System; }

		void Clear();
		bool IsEmpty() const;

		bool Equals( const FilePath& Other, bool bIncludeSystem = true ) const;
		inline bool operator==( const FilePath& Other ) const { return Equals( Other ); }
		inline bool operator!=( const FilePath& Other ) const { return !Equals( Other ); }

		FilePath operator/( const String& toAdd ) const;
		FilePath operator+( const String& toAdd ) const;
		FilePath& operator/=( const String& toAdd );
		FilePath& operator+=( const String& toAdd );

		inline String RootName() const			{ return String( m_Path.root_name().generic_u8string(), StringEncoding::UTF8 ); }
		inline String RootDirectory() const		{ return String( m_Path.root_directory().generic_u8string(), StringEncoding::UTF8 ); }
		inline String RootPath() const			{ return String( m_Path.root_path().generic_u8string(), StringEncoding::UTF8 ); }
		inline String RelativePath() const		{ return String( m_Path.relative_path().generic_u8string(), StringEncoding::UTF8 ); }
		inline String ParentPath() const		{ return String( m_Path.parent_path().generic_u8string(), StringEncoding::UTF8 ); }
		inline String Filename() const			{ return String( m_Path.filename().generic_u8string(), StringEncoding::UTF8 ); }
		inline String Stem() const				{ return String( m_Path.stem().generic_u8string(), StringEncoding::UTF8 ); }
		inline String Extension() const			{ return String( m_Path.extension().generic_u8string(), StringEncoding::UTF8 ); }

		bool HasRootPath() const			{ return m_Path.has_root_path(); }
		bool HasRootName() const			{ return m_Path.has_root_name(); }
		bool HasRootDirectory() const		{ return m_Path.has_root_directory(); }
		bool HasRelativePath() const		{ return m_Path.has_relative_path(); }
		bool HasParentPath() const			{ return m_Path.has_parent_path(); }
		bool HasFilename() const			{ return m_Path.has_filename(); }
		bool HasExtension() const			{ return m_Path.has_extension(); }
		bool HasStem() const				{ return m_Path.has_stem(); }

		inline bool IsDisk() const		{ return m_System == FileSystem::Disk || m_System == FileSystem::None; }
		inline bool IsVirtual() const	{ return m_System == FileSystem::Virtual || m_System == FileSystem::None; }
		inline bool IsNetwork() const	{ return m_System == FileSystem::Network || m_System == FileSystem::None; }

		void SetSystem( FileSystem In );

		friend std::ostream& operator<<( std::ostream&, const FilePath& );
		friend class PhysicalDirectory;
		friend class PhysicalFileSystem;

	};

	template<>
	String ToString( const FilePath& inPath );

	/*
		Function to write file path to output stream so we can print them while debugging
	*/
	std::ostream& operator<<( std::ostream&, const FilePath& );

}