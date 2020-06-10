/*-------------------------------------------------------------------------------------------------
	Hyperion - Texture Library
	© 2020 - Zack Berry
---------------------------------------------------------------------------------------------------*/

#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;


namespace Hyperion
{

	public enum class DXT5Result
	{
		Success = 0,
		InvalidParams = 1,
		PitchCalcError = 2,
		OtherError = 3
	};


	public ref class DXT5Library
	{

	public:

		static DXT5Result Encode( cli::array< Byte >^ inData, UInt32 inWidth, UInt32 inHeight, [ Out ] cli::array< Byte >^% outData, [ Out ] UInt32% outRowSize );
		static DXT5Result DXT5Library::Decode( cli::array< Byte >^ inData, UInt32 inWidth, UInt32 inHeight, UInt32 inPitch, [ Out ] cli::array< Byte >^% outData );

	};

}
