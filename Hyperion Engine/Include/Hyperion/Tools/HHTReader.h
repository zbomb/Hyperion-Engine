/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Tools/HHTReader.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/File/PhysicalFileSystem.h"


namespace Hyperion
{

	class HHTReader
	{

	private:

		DataReader m_Reader;

	public:

		HHTReader( PhysicalFile& inFile );
		~HHTReader();

		HHTReader() = delete;
		HHTReader( const HHTReader& ) = delete;
		HHTReader( HHTReader&& ) = delete;
		HHTReader& operator=( const HHTReader& ) = delete;
		HHTReader& operator=( HHTReader&& ) = delete;

		enum class Result
		{
			Success = 0,
			End = 1,
			Error = 2
		};

		bool Validate();
		void Begin();

		bool NextEntry();
		Result ReadEntry( uint32& outHash, std::vector< byte >& outData );


	};

}