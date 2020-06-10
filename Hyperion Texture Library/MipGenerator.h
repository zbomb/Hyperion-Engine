/*-------------------------------------------------------------------------------------------------
	Hyperion - Texture Library
	© 2020 - Zack Berry
---------------------------------------------------------------------------------------------------*/

#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;


namespace Hyperion
{

	public enum class MipGeneratorResult
	{
		Success = 0,
		InvalidSourceData = 1,
		InvalidSourceResolution = 2,
		GenerateFailed = 3,
		Failed = 255
	};

	public ref class MipGenerator
	{

	public:

		ref struct Request
		{
			cli::array< Byte >^ Data;
			UInt32 Width;
			UInt32 Height;

			// TO ADD: Other Parameters
		};

		ref struct MIP
		{
			Byte Index;
			UInt32 Width;
			UInt32 Height;
			cli::array< Byte >^ Data;
		};

		// We need a way to pass MIP levels in, we know most of the parameters about how the levels are stored, just need data and resolution
		// So, lets create a structure as  input? This would make it a bit easier to pass parameters in and out?
		// Eventually we will have options im sure, for now we just need maybe a structure as input, and a seperate output list
		static MipGeneratorResult Generate( Request^ inParams, [Out] Dictionary< Byte, MIP^ >^% outData );

	};

}
