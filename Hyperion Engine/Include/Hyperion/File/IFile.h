/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/File/IFile.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Stream.h"
#include "Hyperion/File/FilePath.h"



namespace Hyperion
{


	class IFile : public IDataSource
	{

	protected:

		const FilePath m_Path;

		IFile( const FilePath& inPath )
			: m_Path( inPath )
		{}

	public:

		IFile() = delete;

		virtual bool IsValid() const = 0;
		virtual size_t GetSize() const = 0;

		inline const FilePath& GetPath() const { return m_Path; }
	};

}