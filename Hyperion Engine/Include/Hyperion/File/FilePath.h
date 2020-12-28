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

	enum class PathRoot
	{
		SystemRoot		= 0,
		Game			= 1,
		Documents		= 2,
		Content			= 3
	};


	class FilePath
	{

	private:

		PathRoot m_Root;
		std::filesystem::path m_Path;

		FilePath( const std::filesystem::path& inPath, PathRoot inRoot );
		void _Verify();

	public:

		FilePath();
		explicit FilePath( const String& inPath, PathRoot inRoot = PathRoot::Game );
		explicit FilePath( PathRoot inRoot );
		inline FilePath( const char* inPath, PathRoot inRoot = PathRoot::Game ) : FilePath( String( inPath ), inRoot ) {}

		FilePath( const FilePath& Other );
		FilePath( FilePath&& Other ) noexcept;

		FilePath& operator=( const FilePath& Other );
		FilePath& operator=( FilePath&& Other ) noexcept;

		String ToString( bool bAbsolutePath = false ) const;
		std::filesystem::path ToCPath( bool bAbsolutePath = false ) const;

		FilePath ToRootPath() const;

		inline bool IsSystemRootPath() const { return m_Root == PathRoot::SystemRoot; }
		inline PathRoot GetRootPath() const { return m_Root; }

		void Clear();
		bool IsEmpty() const;

		bool Equals( const FilePath& Other ) const;
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

		friend std::ostream& operator<<( std::ostream&, const FilePath& );

		friend class FileSystem;
		friend class Directory;

	};

	template<>
	String ToString( const FilePath& inPath );

	/*
		Function to write file path to output stream so we can print them while debugging
	*/
	std::ostream& operator<<( std::ostream&, const FilePath& );

}