using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;


namespace Hyperion
{

	public struct TextureLOD
	{
		public uint Width;
		public uint Height;
		public uint FileOffset;
		public uint RowSize;
		public byte[] Data;
	}

	public enum TextureFormat
	{
		NONE = 0,

		// Uncompressed Types
		R_8BIT_UNORM			= 1,
		RG_8BIT_UNORM			= 2,
		RGBA_8BIT_UNORM			= 4,
		RGBA_8BIT_UNORM_SRGB	= 5,

		// Compressed Types
		RGB_DXT_1	= 60,
		RGBA_DXT_5	= 61,
		RGB_BC_7	= 62,
		RGBA_BC_7	= 63
	}

	public class Texture
	{
		public TextureFormat Format { get; private set; }
		public byte LODPadding { get; private set; }
		public TextureLOD[] LODs { get; private set; }

		public string Path { get; private set; }
		public uint Hash { get; private set; }

		public Texture( string inPath, uint inHash, TextureFormat inFormat, byte inLODPadding, int inLODCount )
		{
			Path = inPath;
			Hash = inHash;
			Format = inFormat;
			LODPadding = inLODPadding;
			LODs = new TextureLOD[ inLODCount ];
		}
	}

	static class TextureManager
	{

		private static TextureImporter m_ImportUI;

		public static void RegisterConsoleCommands()
		{
			CommandInfo importCommand = new CommandInfo
			{
				Base = "texture_import",
				Usage = "texture_import",
				Description = "Opens the texture importer UI",
				MinArgs = 0,
				MaxArgs = 0,
				Callback = (a) => OpenImportUI()
			};

			Core.RegisterCommand( importCommand );
		}

		public static void OpenImportUI()
		{
			if( m_ImportUI != null )
			{
				if( m_ImportUI.Visible )
				{
					return;
				}

				m_ImportUI = null;
			}

			m_ImportUI = new TextureImporter();
			m_ImportUI.ShowDialog( Program.GetWindow() );
		}

		public static Texture ReadTexture( string inPath )
		{
			// Validate and convert the path to .exe local, from content local
			if( ( inPath?.Length ?? 0 ) == 0 || !inPath.EndsWith( ".htx" ) )
			{
				Core.WriteLine( "[Warning] TextureManager: Failed to load texture, null/invalid path" );
				return null;
			}

			string contentPath;
			if( inPath.StartsWith( "content/" ) )
			{
				contentPath = inPath.Substring( 8, inPath.Length - 8 );
			}
			else
			{
				contentPath = inPath;
				inPath = "content/" + inPath;
			}

			// We also want to calculate the asset identifier for this texture
			uint assetId = Core.GetManifestManager()?.CalculateAssetId( contentPath ) ?? 0;
			if( assetId == 0 )
			{
				Core.WriteLine( "[Warning] TextureManager: Failed to load texture \"", inPath, "\", couldnt calculate asset id" );
				return null;
			}

			FileStream fStream = null;
			try
			{
				fStream = File.Open( inPath, FileMode.Open );
				if( fStream == null ) throw new Exception( "File was null" );
			}
			catch( Exception Ex )
			{
				Core.WriteLine( "[Warning] TextureManager: Failed to load texture \"", inPath, "\", file couldnt be opened (", Ex.Message, ")" );
				fStream?.Dispose();
				return null;
			}

			// Now lets read the header data in, and validate
			var headerData = new byte[ 20 ];
			try
			{
				if( fStream.Read( headerData, 0, 20 ) != 20 )
				{
					throw new Exception( "Failed to read from file" );
				}

				var validHeader = new byte[]{ 0x1A, 0xA1, 0xFF, 0x28, 0x9D, 0xD9, 0x00, 0x02 };
				for( int i = 0; i < validHeader.Length; i++ )
				{
					if( headerData[ i ] != validHeader[ i ] )
					{
						throw new Exception( "Invalid header sequence" );
					}
				}
			}
			catch( Exception Ex )
			{
				Core.WriteLine( "[Warning] TextureManager: Failed to read texture \"", inPath, "\" (", Ex.Message, ")" );
				fStream?.Dispose();
				return null;
			}

			// Deserialize the header values we need
			byte headerNum		= Serialization.GetUInt8( headerData, 8 );
			byte lodPadding		= Serialization.GetUInt8( headerData, 9 );
			byte lodCount		= Serialization.GetUInt8( headerData, 10 );

			if( !Enum.IsDefined( typeof( TextureFormat ), headerNum ) || lodCount > 15 || headerNum == 0 )
			{
				Core.WriteLine( "[Warning] TextureManager: failed to read texture \"", inPath, "\" because the header values were out of range" );
				fStream?.Dispose();
				return null;
			}

			var Output = new Texture( inPath, assetId, (TextureFormat) headerNum, lodPadding, lodCount );

			// Now we can calculate how long the files need to be to hold the LOD data
			var lodListData		= new byte[ lodCount * 16 ];
			try
			{
				if( fStream.Read( lodListData, 0, lodListData.Length ) != lodListData.Length )
				{
					throw new Exception( "Failed to read from file" );
				}
			}
			catch( Exception Ex )
			{
				Core.WriteLine( "[Warning] TextureManager: Failed to read texture LOD List \"", inPath, "\" (", Ex.Message, ")" );
				fStream?.Dispose();
				return null;
			}

			for( byte i = 0; i < lodCount; i++ )
			{
				var lodData = new byte[ 16 ];
				try
				{
					if( fStream.Read( lodData, 0, 16 ) != 16 )
					{
						throw new Exception( "Failed to read file" );
					}
				}
				catch( Exception Ex )
				{
					Core.WriteLine( "[Warning] TextureManager: Failed to read texture LOD \"", inPath, "\" (", Ex.Message, ")" );
					fStream?.Dispose();
					return null;
				}

				// Deserialize info about this LOD
				Output.LODs[ i ].Width = Serialization.GetUInt16( lodData, 0, false );
				Output.LODs[ i ].Height = Serialization.GetUInt16( lodData, 2, false );
				Output.LODs[ i ].FileOffset = Serialization.GetUInt32( lodData, 4, false );

				uint lodSize = Serialization.GetUInt32( lodData, 8, false );
				Output.LODs[ i ].Data = new byte[ lodSize ];
				Output.LODs[ i ].RowSize = Serialization.GetUInt32( lodData, 12, false );

				// Lets do a quick validation of this
				if( lodSize == 0 || Output.LODs[ i ].RowSize == 0 || Output.LODs[ i ].Width == 0 || Output.LODs[ i ].Height == 0 )
				{
					Core.WriteLine( "[Warning] TextureManager: Failed to read texture LOD ", (uint) i, " in \"", inPath, "\" because the values were invalid" );
					fStream?.Dispose();
					return null;
				}

				// And finally, load the pixel data into the LOD
				try
				{
					fStream.Seek( Output.LODs[ i ].FileOffset, SeekOrigin.Begin );
					if( fStream.Read( Output.LODs[ i ].Data, 0, (int)lodSize ) != (int)lodSize )
					{
						throw new Exception( "Failed to read file" );
					}
				}
				catch( Exception Ex )
				{
					Core.WriteLine( "[Warning] TextureManager: Failed to read texture LOD ", (uint) i, " in \"", inPath, "\" (", Ex.Message, ")" );
					fStream?.Dispose();
					return null;
				}
			}

			fStream?.Dispose();
			return Output;
		}


		/*
		 *	TextureManager::SaveTexture
		 *	- We dont save textures like we do in the material system, where we can open and edit them
		 *	- This is used to import and image, fill out a 'Texture' object, and pass it here to write it to file
		 */
		public static bool SaveTexture( Texture inTexture )
		{
			// Validate the input texture quickly
			if( ( inTexture?.Path?.Length ?? 0 ) == 0 || ( inTexture?.Hash ?? 0 ) == 0 || inTexture.Format == TextureFormat.NONE 
				|| !inTexture.Path.EndsWith( ".htx" ) )
			{
				Core.WriteLine( "[Warning] TextureManager: Failed to save texture, invalid path/hash/format" );
				return false;
			}

			if( inTexture.LODs.Length > 15 )
			{
				Core.WriteLine( "[Warning] TextureManager: Failed to save texture, too many LOD levels!" );
				return false;
			}

			// This is tricky to write, since we need file offsets for the different LOD levels, luckily, we can calculate this using....
			// headerSize + ( lodCount * lodSize ) + Sum( LODData from level 0 to this level )
			var fileData = new List< byte >{ 0x1A, 0xA1, 0xFF, 0x28, 0x9D, 0xD9, 0x00, 0x02 };
			fileData.Add( (byte) inTexture.Format );
			fileData.Add( (byte) inTexture.LODPadding );
			fileData.Add( (byte) inTexture.LODs.Length );
			fileData.Add( 0 );
			fileData.AddRange( new byte[]{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } );

			var pixelData			= new List< byte >();
			var staticDataLength	= 20 + ( inTexture.LODs.Length * 16 );

			for( int i = 0; i < inTexture.LODs.Length; i++ )
			{
				fileData.AddRange( Serialization.FromUInt16( (ushort) inTexture.LODs[ i ].Width, false ) );
				fileData.AddRange( Serialization.FromUInt16( (ushort) inTexture.LODs[ i ].Height, false ) );
				fileData.AddRange( Serialization.FromUInt32( (uint)( staticDataLength + pixelData.Count ), false ) );
				fileData.AddRange( Serialization.FromUInt32( (uint) inTexture.LODs[ i ].Data.Length, false ) );
				fileData.AddRange( Serialization.FromUInt32( (uint) inTexture.LODs[ i ].RowSize, false ) ); // Maybe we should calculate this in this function?

				pixelData.AddRange( inTexture.LODs[ i ].Data );
			}

			fileData.AddRange( pixelData );

			// Now that we have the file built, we need to write it to file
			bool bNewFile		= false;
			FileStream fStream	= null;

			string fullPath;
			string relPath;
			if( inTexture.Path.StartsWith( "content/" ) )
			{
				fullPath = inTexture.Path;
				relPath = inTexture.Path.Substring( 8 );
			}
			else
			{
				fullPath = "content/" + inTexture.Path;
				relPath = inTexture.Path;
			}

			try
			{
				if( File.Exists( fullPath ) )
				{
					fStream = File.Open( fullPath, FileMode.Truncate );
				}
				else
				{
					fStream = File.Create( fullPath );
					bNewFile = true;
				}

				if( fStream == null ) throw new Exception( "File was null" );

				fStream.Write( fileData.ToArray(), 0, fileData.Count );
			}
			catch( Exception Ex )
			{
				Core.WriteLine( "[Warning] TextureManager: Failed to write texture \"", inTexture.Path, "\" (", Ex.Message, ")" );
				fStream?.Dispose();
				return false;
			}

			if( bNewFile )
			{
				if( !( Core.GetManifestManager()?.AddEntry( inTexture.Hash, relPath ) ?? false ) )
				{
					Core.WriteLine( "[Warning] TextureManager: Wrote texture \"", inTexture.Path, "\" but failed to add new asset id to the manifest!" );
				}
			}

			fStream?.Dispose();
			return true;
		}

	}
}
