using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows.Media.Imaging;
using System.Windows.Media;
using TGASharpLib;
using System.Runtime.InteropServices;

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

		private static bool IsRGB( ref BitmapSource inSource )
		{
			return ( inSource.Format == PixelFormats.Bgr24 ||
				inSource.Format == PixelFormats.Bgr32 ||
				inSource.Format == PixelFormats.Bgr101010 ||
				inSource.Format == PixelFormats.Bgr555 ||
				inSource.Format == PixelFormats.Bgr565 ||
				inSource.Format == PixelFormats.Rgb128Float ||
				inSource.Format == PixelFormats.Rgb24 ||
				inSource.Format == PixelFormats.Rgb48 );
		}

		private static bool IsRGBA( ref BitmapSource inSource )
		{
			return (
				inSource.Format == PixelFormats.Pbgra32 ||
				inSource.Format == PixelFormats.Prgba64 ||
				inSource.Format == PixelFormats.Bgra32 ||
				inSource.Format == PixelFormats.Prgba128Float ||
				inSource.Format == PixelFormats.Rgba128Float ||
				inSource.Format == PixelFormats.Rgba64 );
		}

		private static bool IsGrayscale( ref BitmapSource inSource )
		{
			return (
				inSource.Format == PixelFormats.Gray16 ||
				inSource.Format == PixelFormats.Gray2 ||
				inSource.Format == PixelFormats.Gray32Float ||
				inSource.Format == PixelFormats.Gray4 ||
				inSource.Format == PixelFormats.Gray8 );
		}

		private static bool IsGrayscaleAlpha( ref BitmapSource inSource )
		{
			return false;
		}

		public static bool AutoImport( string inPath, out RawImageData outData )
		{
			// Set default values
			outData = null;

			/*
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
					return NewPNGImporter.Import( fileData, out outData );
					//return PNGImporter.Import( fileData, 0.0f, out outData );
				case ImporterType.TGA:
					return TGAImporter.Import( fileData, out outData );
				default:
					return false;
			}
			*/

			// Were going to attempt this another way, using .NET to perform the import of the image, since it can do just about everything we need
			// and this way we dont have to write our own importers for each file format

			/*
			Bitmap bmp = null;
			try
			{
				bmp = new Bitmap( inPath );
				if( bmp == null ) { throw new Exception( "New BMP was null!" ); }
			}
			catch( Exception Ex )
			{
				Core.WriteLine( "[Warning] ImageImport: Failed to import image (", inPath, ") because: ", Ex.Message );
				return false;
			}

			outData = new RawImageData();

			// Read the pixel format
			// Issue: The grayscale formats (at least in PNGs) are being tagged as 32bppArgb or 24bppRgb
			// When, we want to be able to decide which channels we pass color info into, the only way to properly
			// make this work, is to somehow figure out when the source is grayscale
			Core.WriteLine( "[DEBUG] Image Flags: ", bmp.Flags );
			

			switch( bmp.PixelFormat )
			{
				case PixelFormat.Format32bppArgb:
					Core.WriteLine( "[DEBUG] PixelFormat: RGBA_8BIT_UNORM" );



					break;
				case PixelFormat.Format24bppRgb:
					Core.WriteLine( "[DEBUG] PixelFormat: RGB_8BIT_UNORM" );

					break;

				default:
					Core.WriteLine( "[DEBUG] PiexelFormat: Unkown.... (", Enum.GetName( bmp.PixelFormat.GetType(), bmp.PixelFormat ), ")" );
					break;
			}
			*/

			// Method 3
			// Presentation Core
			// This gives us more control over the import of images, and better info about the source pixel format
			// The previous method, would automatically convert grayscale images into RGB or RGBA pixel formats, preventing us
			// to have access to the actual underlying pixel format, and in our use case, we want access to control how conversion happens
			if( ( inPath?.Length ?? 0 ) < 3 || !File.Exists( inPath ) )
			{
				Core.WriteLine( "[Warning] Failed to import image.. invalid path provided! (", inPath, ")" );
				return false;
			}

			if( inPath.EndsWith( ".tga" ) )
			{
				try
				{
					// Open the file using the TGA library were using
					var img = new TGASharpLib.TGA( inPath );
					if( img == null ) { throw new Exception( "TGA file couldnt be opened" ); }

					// We use textures with an origin of the top left, so we need to correct for differing origins in the tga source image
					switch( img.Header.ImageSpec.ImageDescriptor.ImageOrigin )
					{
						case TgaImgOrigin.BottomLeft:
							// Flip vertically
							img.Flip( false, true );
							break;
						case TgaImgOrigin.BottomRight:
							// Flip both horizontally and vertically
							img.Flip( true, true );
							break;
						case TgaImgOrigin.TopRight:
							// Flip hotizontally
							img.Flip( true, false );
							break;
						default:
							break;
					}

					var bmp = img.ToBitmap( true );
					if( bmp == null ) { throw new Exception( "TGA file couldnt be converted to bitmap data" ); }

					// DEBUG DEBUG DEBUG
					Core.WriteLine( "[DEBUG] TGA => Bitmap Format: ", bmp.PixelFormat.ToString() );


					// Now, lock the image data and copy it into a managed array so we can access it
					var width = bmp.Size.Width;
					var height = bmp.Size.Height;

					//bool bAlpha = img.Header.ImageSpec.ImageDescriptor.AlphaChannelBits != 0;
					bool bAlpha = true;

					// Determine the source format
					ImageFormat fmt;
					switch( img.Header.ImageType )
					{
						case TgaImageType.NoImageData:

							throw new Exception( "TGA file had 'NoImageData' format!" );

						case TgaImageType.RLE_BlackWhite:
						case TgaImageType.Uncompressed_BlackWhite:

							fmt = bAlpha ? ImageFormat.GSA : ImageFormat.GS;
							break;

						case TgaImageType.RLE_ColorMapped:
						case TgaImageType.Uncompressed_ColorMapped:
						case TgaImageType.RLE_TrueColor:
						case TgaImageType.Uncompressed_TrueColor:

							fmt = bAlpha ? ImageFormat.RGBA : ImageFormat.RGB;
							break;

						default:

							// Default to RGBA
							fmt = bAlpha ? ImageFormat.RGBA : ImageFormat.RGB;
							Core.WriteLine( "[Warning] ImageImporter: Opened TGA file, but, it had an unknown format, defaulting to RGBA" );
							break;
					}

					Core.WriteLine( "[DEBUG] Importing TGA: Format = ", Enum.GetName( typeof( ImageFormat ), fmt ) );

					byte[] copiedData;
					{
						// So, we have a bitmap, we want to convert it to PArgb
						Bitmap convBitmap = new Bitmap( width, height, System.Drawing.Imaging.PixelFormat.Format32bppPArgb );

						using( Graphics gr = Graphics.FromImage( convBitmap ) )
						{
							gr.DrawImage( bmp, new Rectangle( 0, 0, convBitmap.Width, convBitmap.Height ) );
						}

						// Now, we copy the data out of the bitmap...
						var bmpData = convBitmap.LockBits( new Rectangle( 0, 0, width, height ), ImageLockMode.ReadOnly, convBitmap.PixelFormat );
						copiedData = new byte[ bmpData.Stride * height ];
						Marshal.Copy( bmpData.Scan0, copiedData, 0, copiedData.Length );
						convBitmap.UnlockBits( bmpData );
					}

					// We have to perform channel swapping to get the output image correct
					int[] swapMatrix = { 2, 1, 0, 3 };
					if( !ImageUtils.SwapChannels( copiedData, swapMatrix ) )
					{
						Core.WriteLine( "[DEBUG] ImageImporter: Failed to import TGA, channel swap failed!" );
						return false;
					}

					// Now, we have the final data, and we can create the raw image data
					outData = new RawImageData()
					{
						Width = ( uint ) width,
						Height = ( uint ) height,
						Data = copiedData,
						Format = fmt
					};

					return true;
				}
				catch( Exception Ex )
				{
					Core.WriteLine( "[Warning] Failed to import image.. couldnt open the TGA (", Ex.Message, ")" );
					return false;
				}
			}
			else
			{
				var path = new Uri( Path.GetFullPath( inPath ) );
				BitmapDecoder decoder = null;

				try
				{
					decoder = BitmapDecoder.Create( path, BitmapCreateOptions.None, BitmapCacheOption.Default );
					if( decoder == null ) { throw new Exception( "Decoder was null??" ); }
					if( decoder.Frames.Count == 0 ||
						decoder.Frames[ 0 ] == null ) { throw new Exception( "There are no frames in the source image" ); }
				}
				catch( Exception Ex )
				{
					Core.WriteLine( "[Warning] Failed to import iamge.. couldnt create a decoder! (", Ex.Message, ")" );
					return false;
				}

				// Now we need to copy the pixel data into an array, in our desired format
				var fmt = ImageFormat.NONE;
				BitmapSource source = decoder.Frames[ 0 ];

				// Also, we need to figure out the proper format
				if( IsRGBA( ref source ) )
				{
					fmt = ImageFormat.RGBA;
					//Core.WriteLine( "[DEBUG] Format was RGBA" );
				}
				else if( IsRGB( ref source ) )
				{
					fmt = ImageFormat.RGB;
					//Core.WriteLine( "[DEBUG] Format was RGB" );
				}
				else if( IsGrayscale( ref source ) )
				{
					fmt = ImageFormat.GS;
					//Core.WriteLine( "[DEBUG] Format was GS" );
				}
				else if( IsGrayscaleAlpha( ref source ) )
				{
					fmt = ImageFormat.GSA;
					//Core.WriteLine( "[DEBUG] Format was GSA" );
				}
				else
				{
					Core.WriteLine( "[DEBUG] Failed to import image... Unknown Format! ", source.Format.ToString() );
					return false;
				}


				// We want to convert the bitmap to RGBA 32bpp, this way we can more easily access the pixels
				if( source.Format != PixelFormats.Pbgra32 )
				{
					source = new FormatConvertedBitmap( source, PixelFormats.Pbgra32, null, 0.0 );
				}

				// Now, allocate array and copy the data
				outData = new RawImageData();
				outData.Width = ( uint ) source.PixelWidth;
				outData.Height = ( uint ) source.PixelHeight;
				outData.Data = new byte[ source.PixelWidth * source.PixelHeight * 4 ];
				outData.Format = fmt;

				source.CopyPixels( outData.Data, source.PixelWidth * 4, 0 );

				// We want to re-order them as RGBA
				for( uint y = 0; y < outData.Height; y++ )
				{
					for( uint x = 0; x < outData.Width; x++ )
					{
						long memOffset = ( ( y * outData.Width ) + x ) * 4L;

						// Source: Bgra32 [b:0][g:1][r:2][a:3]
						// Target: Rgba32 [r:0][g:1][b:2][a:3]
						// We need to switch r and b, index 0 and 2
						byte sourceR = outData.Data[ memOffset + 2 ];
						outData.Data[ memOffset + 2 ] = outData.Data[ memOffset ];
						outData.Data[ memOffset ] = sourceR;
					}
				}

				return true;
			}
		}
	}
}
