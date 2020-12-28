/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Tools/HHTWriter.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/File/FileSystem.h"


namespace Hyperion
{

	class HHTWriter
	{

	private:

		File& m_File;
		DataWriter m_Writer;
		bool m_bHeaderFailed;

		std::map< uint32, std::vector< byte > > m_Entries;

		void _WriteHeader();

	public:

		HHTWriter( File& inFile, bool bIsNewFile  /* = true */ );
		~HHTWriter();

		HHTWriter() = delete;
		HHTWriter( const HHTWriter& ) = delete;
		HHTWriter( HHTWriter&& ) = delete;
		HHTWriter& operator=( const HHTWriter& ) = delete;
		HHTWriter& operator=( HHTWriter&& ) = delete;

		void ClearList();
		void AddEntry( uint32 hashData, const std::vector< byte >& valueData );

		enum class Result
		{
			Success = 0,
			SizeOverflow = 1,
			WriteFailed = 2,
			Failed = 3
		};

		Result Flush();
	};

}