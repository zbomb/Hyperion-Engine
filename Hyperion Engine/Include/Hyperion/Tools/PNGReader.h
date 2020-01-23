/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Tools/PNGReader.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Types/ITexture.h"


namespace Hyperion
{
namespace Tools
{
	/*
		Forward Declarations
	*/
	//struct RawImageData;


	class PNGReader
	{

	public:

		PNGReader() = delete;

		enum class Result
		{
			Failed = 0,
			Success = 1,
			NotPNG = 2,
			BadChunks = 3,
			InvalidChunks = 4,
			InvalidIHDR = 5,
			InvalidPLTE = 6,
			InvalidIDAT = 7
		};


		static Result LoadFromMemory( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, std::shared_ptr< RawImageData >& Out, float ScreenGamma = 1.f );

	};

}
}