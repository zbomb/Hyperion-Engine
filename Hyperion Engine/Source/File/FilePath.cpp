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
		: m_Root( PathRoot::Game ), m_Path()
	{}


	FilePath::FilePath( const std::filesystem::path& inPath, PathRoot inRoot )
		: m_Path( inPath ), m_Root( inRoot )
	{
		_Verify();
	}


	FilePath::FilePath( const String& inPath, PathRoot inRoot /* = PathRoot::Game */ )
		: m_Path( inPath.GetU8Str() ), m_Root( inRoot )
	{
		_Verify();
	}


	FilePath::FilePath( PathRoot inRoot )
		: m_Path(), m_Root( inRoot )
	{
		_Verify();
	}


	FilePath::FilePath( const FilePath& Other )
		: FilePath( Other.m_Path, Other.m_Root )
	{}


	FilePath::FilePath( FilePath&& Other ) noexcept
		: m_Path( std::move( Other.m_Path ) ), m_Root( std::move( Other.m_Root ) )
	{
		_Verify();
		Other.Clear();
	}

	FilePath& FilePath::operator=( const FilePath& Other )
	{
		m_Path		= Other.m_Path;
		m_Root		= Other.m_Root;

		_Verify();

		return *this;
	}


	FilePath& FilePath::operator=( FilePath&& Other ) noexcept
	{
		m_Path		= std::move( Other.m_Path );
		m_Root		= std::move( Other.m_Root );

		_Verify();

		return *this;
	}


	void FilePath::_Verify()
	{
		// DEPRECATED: TODO: Remove
	}


	String FilePath::ToString( bool bAbsolutePath /* = false */ ) const
	{
		String relativePath( m_Path.generic_u8string(), StringEncoding::UTF8 );
		return bAbsolutePath ? IFileServices::Get().GetRootPathLocation( m_Root ).Append( relativePath ) : relativePath;
	}


	std::filesystem::path FilePath::ToCPath( bool bAbsolutePath /* = false */ ) const
	{
		return std::filesystem::path( ToString( bAbsolutePath ).GetU8Str() );
	}


	FilePath FilePath::ToRootPath() const
	{
		return m_Root == PathRoot::SystemRoot ? FilePath( *this ) : FilePath( ToString( true ), PathRoot::SystemRoot );
	}


	void FilePath::Clear()
	{
		m_Root		= PathRoot::Game;
		m_Path.clear();
	}


	bool FilePath::IsEmpty() const
	{
		return m_Path.empty();
	}


	bool FilePath::Equals( const FilePath& Other ) const
	{
		return m_Root == Other.m_Root && ( m_Path.compare( Other.m_Path ) == 0  );
	}


	FilePath FilePath::operator/( const String& toAppend ) const
	{
		std::filesystem::path pathCopy( m_Path );
		std::filesystem::path newPart( toAppend.GetU8Str() );

		pathCopy /= newPart;

		return FilePath( pathCopy, m_Root );
	}


	FilePath FilePath::operator+( const String& toAppend ) const
	{
		std::filesystem::path pathCopy( m_Path );
		std::filesystem::path newPart( toAppend.GetU8Str() );

		pathCopy += newPart;

		return FilePath( pathCopy, m_Root );
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


	template<>
	String ToString( const FilePath& inPath )
	{
		return inPath.ToString();
	}


	std::ostream& operator<<( std::ostream& inStream, const FilePath& inPath )
	{
		return inStream << inPath.ToCPath();
	}


}