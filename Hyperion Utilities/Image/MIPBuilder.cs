using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Hyperion
{
	class MIPBuilder
	{

		public enum Result
		{
			Success = 0,
			SourceTooSmall = 1,
			Failed = 2
		}

		public static Result Generate( ref RawImageData inSource, out List< RawImageData > outLevels )
		{
			outLevels = null;

			// Now, what we have to do is take the source image, and generate a mip chain
			// All inputs and outputs are RGBA_8BIT_UNORM type image data
			if( inSource == null ) { return Result.Failed; }
			if( inSource.Width < 1 || inSource.Height < 1 ) { return Result.SourceTooSmall; }
			if( inSource.Width == 1 && inSource.Height == 1 ) { return Result.SourceTooSmall; }

			MipGenerator.Request req = new MipGenerator.Request()
			{
				Width = inSource.Width,
				Height = inSource.Height,
				Data = inSource.Data
			};

			// Generate the mip levels
			var res = MipGenerator.Generate( req, out Dictionary< byte, MipGenerator.MIP > outData );
			switch( res )
			{
				case MipGeneratorResult.Success:
					break;
				default:
					Core.WriteLine( "[Warning] MipBuilder: Failed to build mips for input image.. error (", Enum.GetName( typeof( MipGeneratorResult ), res ) );
					return Result.Failed;
			}

			// Now, we need to convert what we got into the output type we need
			outLevels = new List< RawImageData >( outData.Count );
			while( outLevels.Count < outData.Count )
			{
				outLevels.Add( new RawImageData() );
			}

			foreach( var pair in outData )
			{
				outLevels[ pair.Value.Index ].Data = pair.Value.Data;
				outLevels[ pair.Value.Index ].Width = pair.Value.Width;
				outLevels[ pair.Value.Index ].Height = pair.Value.Height;
				outLevels[ pair.Value.Index ].Format = inSource.Format;
			}

			return Result.Success;
		}

	}
}
