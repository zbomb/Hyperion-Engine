/*==================================================================================================
	Hyperion Engine
	Source/File/FilePath.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/File/FilePath.h"
#include "Hyperion/File/IFileServices.h"


namespace Hyperion
{

	FilePath::FilePath()
		: m_Local( LocalPath::Game ), m_System( FileSystem::None ), m_Path()
	{}


	FilePath::FilePath( const std::filesystem::path& inPath, LocalPath inLocal, FileSystem inSystem )
		: m_Path( inPath ), m_Local( inLocal ), m_System( inSystem )
	{
		_Verify();
	}


	FilePath::FilePath( const String& inPath, LocalPath inLocal /* = LocalPath::Game */, FileSystem inSystem /* = FileSystem::None */ )
		: m_Path( inPath.GetU8Str() ), m_Local( inLocal ), m_System( inSystem )
	{
		_Verify();
	}


	FilePath::FilePath( LocalPath inLocal, FileSystem inSystem /* = FileSystem::None */ )
		: m_Path(), m_Local( inLocal ), m_System( inSystem )
	{
		_Verify();
	}


	FilePath::FilePath( const String& inPath, FileSystem inSystem )
		: m_Path( inPath.GetU8Str() ), m_System( inSystem ), m_Local( LocalPath::Game )
	{
		_Verify();
	}


	FilePath::FilePath( const FilePath& Other )
		: FilePath( Other.m_Path, Other.m_Local, Other.m_System )
	{}


	FilePath::FilePath( FilePath&& Other ) noexcept
		: m_Path( std::move( Other.m_Path ) ), m_Local( std::move( Other.m_Local ) ), m_System( std::move( Other.m_System ) )
	{
		_Verify();
		Other.Clear();
	}

	FilePath& FilePath::operator=( const FilePath& Other )
	{
		m_Path		= Other.m_Path;
		m_Local		= Other.m_Local;
		m_System	= Other.m_System;

		_Verify();

		return *this;
	}


	FilePath& FilePath::operator=( FilePath&& Other ) noexcept
	{
		m_Path		= std::move( Other.m_Path );
		m_Local		= std::move( Other.m_Local );
		m_System	= std::move( Other.m_System );

		_Verify();

		return *this;
	}


	void FilePath::_Verify()
	{
		// There are a couple local paths only accessible by the physical file system, so lets ensure there isnt a mismatch here
		if( ( m_Local == LocalPath::Root || m_Local == LocalPath::Data ) && ( m_System == FileSystem::Network || m_System == FileSystem::Virtual ) )
		{
			Console::WriteLine( "[WARNING] FileSystem: Invalid file path! Can only access 'root' and 'data' files on disk! Defaulting to disk..." );
			m_System = FileSystem::Disk;
		}
	}


	String FilePath::ToString( bool bAbsolutePath /* = false */ ) const
	{
		String relativePath( m_Path.generic_u8string(), StringEncoding::UTF8 );
		return bAbsolutePath ? IFileServices::Get().GetLocalPathLocation( m_Local ).Append( relativePath ) : relativePath;
	}


	std::filesystem::path FilePath::ToCPath( bool bAbsolutePath /* = false */ ) const
	{
		return std::filesystem::path( ToString( bAbsolutePath ).GetU8Str() );
	}


	FilePath FilePath::ToRootPath() const
	{
		return m_Local == LocalPath::Root ? FilePath( *this ) : FilePath( ToString( true ), LocalPath::Root, m_System );
	}


	void FilePath::Clear()
	{
		m_System	= FileSystem::None;
		m_Local		= LocalPath::Game;
		m_Path.clear();
	}


	bool FilePath::IsEmpty() const
	{
		return m_Path.empty();
	}


	bool FilePath::Equals( const FilePath& Other, bool bIncludeSystem /* = true */ ) const
	{
		return m_Local == Other.m_Local && ( m_Path.compare( Other.m_Path ) == 0 ) && ( !bIncludeSystem || m_System == Other.m_System );
	}


	FilePath FilePath::operator/( const String& toAppend ) const
	{
		std::filesystem::path pathCopy( m_Path );
		std::filesystem::path newPart( toAppend.GetU8Str() );

		pathCopy /= newPart;

		return FilePath( pathCopy, m_Local, m_System );
	}


	FilePath FilePath::operator+( const String& toAppend ) const
	{
		std::filesystem::path pathCopy( m_Path );
		std::filesystem::path newPart( toAppend.GetU8Str() );

		pathCopy += newPart;

		return FilePath( pathCopy, m_Local, m_System );
	}


	FilePath& FilePath::operator/=( const String& toAppend )
	{
		std::filesystem::path newPart( toAppend.GetU8Str() );
		m_Path /= newPart;

		return *this;
	}


	FilePath& FilePath::operator+=( const String& toAppend )
	{
		std::filesystem::path newPart( toAppend.GetU8Str() );
		m_Path += newPart;

		return *this;
	}


	void FilePath::SetSystem( FileSystem In )
	{
		m_System = In;
		_Verify();
	}


	std::ostream& operator<<( std::ostream& inStream, const FilePath& inPath )
	{
		return inStream << inPath.ToCPath();
	}


}