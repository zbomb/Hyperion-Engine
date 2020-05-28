using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;


namespace Hyperion
{
	public enum ImageFormat
	{
		NONE = 0,
		GS = 1,
		GSA = 2,
		RGB = 3,
		RGBA = 4
	}


	public class RawImageData
	{
		public ImageFormat Format;
		public uint Width;
		public uint Height;
		public byte[] Data;
	}


	static class GenericImporter
	{
		private enum ImporterType
		{
			PNG,
			TGA,
			JPG
		}

		public static bool AutoImport( string inPath, out RawImageData outData )
		{
			// Set default values
			outData = null;

			// Validate the path
			if( ( inPath?.Length ?? 0 ) < 4 )
			{
				Core.WriteLine( "[Warning] ImageImport: Failed to import image (", inPath ?? "", ") because the path was null or too short" );
				return false;
			}

			ImporterType importType;
			var ext = Path.GetExtension( inPath ).ToLower();

			if( ext == ".png" )
			{
				importType = ImporterType.PNG;
			}
			else if( ext == ".tga" )
			{
				importType = ImporterType.TGA;
			}
			else if( ext == ".jpg" )
			{
				importType = ImporterType.JPG;
			}
			else
			{
				Core.WriteLine( "[Warning] ImageImport: Failed to import image (", inPath, ") because the extension was unknown" );
				return false;
			}

			// Now, we want to read the whole file into a byte array
			byte[] fileData = null;

			try
			{
				var f = File.OpenRead( inPath );
				if( f == null ) { throw new Exception( "File was null" ); }

				f.Seek( 0, SeekOrigin.Begin );
				fileData = new byte[ f.Length ];
				if( f.Read( fileData, 0, (int)f.Length ) != (int) f.Length )
				{
					throw new Exception( "Failed to read all file data" );
				}
			}
			catch( Exception Ex )
			{
				Core.WriteLine( "[Warning] ImageImport: Failed to import image (", inPath, ") because the file couldnt be read [", Ex.Message, "]" );
				return false;
			}

			switch( importType )
			{
				case ImporterType.JPG:
					return JPGImporter.Import( fileData, out outData );
				case ImporterType.PNG:
					return PNGImporter.Import( fileData, 0.0f, out outData );
				case ImporterType.TGA:
					return TGAImporter.Import( fileData, out outData );
				default:
					return false;
			}
		}

	}
}
