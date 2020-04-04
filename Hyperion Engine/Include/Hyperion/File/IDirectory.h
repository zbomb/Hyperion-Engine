/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/File/IFile.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


#include "Hyperion/Hyperion.h"


namespace Hyperion
{

	class IDirectory
	{

	protected:

		IDirectory( const FilePath& inPath, bool inValid )
			: m_Path( inPath ), m_Valid( inValid ), m_Cached( false )
		{}

		FilePath m_Path;
		bool m_Valid;
		
		std::vector< FilePath > m_Files;
		std::vector< FilePath > m_Directories;

		bool m_Cached;

		virtual void CacheContents() = 0;

	public:

		IDirectory() = delete;
		IDirectory( const IDirectory& ) = delete;

		inline const FilePath& GetPath() const { return m_Path; }
		inline bool IsValid() const { return m_Valid; }

		const std::vector< FilePath >& GetSubFiles()
		{
			if( !m_Cached ) 
			{ 
				CacheContents(); 
				m_Cached = true; 
			}

			return m_Files;
		}

		const std::vector< FilePath >& GetSubDirectories()
		{
			if( !m_Cached )
			{
				CacheContents();
				m_Cached = true;
			}

			return m_Directories;
		}

	};

}