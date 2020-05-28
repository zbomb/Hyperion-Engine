/*-------------------------------------------------------------------------------------------------
	Hyperion - Texture Library
	© 2020 - Zack Berry
---------------------------------------------------------------------------------------------------*/

#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;


namespace Hyperion
{

	public enum class DXT1Result
	{
		Success = 0,
		InvalidParams = 1,
		OtherError = 2
	};


	public ref class DXT1Library
	{

	public:

		static DXT1Result Encode( cli::array< Byte >^ inData, UInt32 inWidth, UInt32 inHeight, [ Out ] cli::array< Byte >^% outData, [ Out ] UInt32% outRowSize );

	};

}