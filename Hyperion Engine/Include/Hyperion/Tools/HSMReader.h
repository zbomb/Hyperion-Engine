/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Tools/HSMReader.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/File/IFile.h"
#include "Hyperion/Assets/StaticModelAsset.h"


namespace Hyperion
{
	class HSMReader
	{

	public:

		HSMReader() = delete;

		enum Result
		{
			Success = 0,
			InvalidFile = 1,
			InvalidVersion = 2,
			MissingData = 3,
			InvalidMaterial = 4,
			InvlaidLOD = 5,
			InvalidObject = 6,
			Failed = 10
		};

		static const uint16 ToolsetVersion = 0x0001;
		static Result ReadFile( IFile& inFile, std::shared_ptr< StaticModelAsset >& outAsset );

	};

}