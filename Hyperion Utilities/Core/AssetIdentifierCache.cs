using System;
using System.Collections.Generic;
using System.Xml;
using System.Xml.Serialization;
using System.IO;


namespace Hyperion
{
	class AssetIdentifierCache
	{
		private static Dictionary< uint, string > m_AssetList = new Dictionary< uint, string >();


		public static Dictionary<uint, string> GetList() => m_AssetList;

		public static bool AssetExists( uint inIdentifier )
		{
			if( inIdentifier == 0 ) { return false; }

			return m_AssetList.ContainsKey( inIdentifier );
		}


		public static void PerformDiscovery()
		{
			var fileList = Directory.GetFiles( "content/", "*", SearchOption.AllDirectories );
			foreach( var f in fileList )
			{
				// Cleanup the path
				string nf = f.Replace( "\\\\", "/" ).Replace( "\\", "/" );
				string cleanPath;

				if( nf.StartsWith( "content/" ) || nf.StartsWith( "Content/" ) )
				{
					cleanPath = nf.Substring( 8 );
				}
				else
				{
					cleanPath = nf;
				}

				// Calculate the identifier
				uint id = Core.CalculateAssetIdentifier( cleanPath );

				// Add into the asset list
				m_AssetList[ id ] = cleanPath;
			}

			Core.WriteLine( "AssetCache: Finished asset discovery! Found ", fileList.Length, " assets." );
		}


		public static bool RegisterAsset( string inPath )
		{
			// Validate the parameters
			if( String.IsNullOrWhiteSpace( inPath ) || inPath.Length < 4 )
			{
				Core.WriteLine( "[Warning] AssetIdentifierCache: Failed to register asset, invalid parameters" );
				return false;
			}

			// Ensure the begining of the path doesnt include the content directory
			inPath = inPath.Replace( "\\\\", "/" ).Replace( "\\", "/" );
			if( inPath.StartsWith( "content/" ) || inPath.StartsWith( "Content/" ) )
			{
				inPath = inPath.Substring( 8 );
			}

			uint id = Core.CalculateAssetIdentifier( inPath );
			m_AssetList[ id ] = inPath;

			return true;
		}


		public static bool RemoveAsset( uint inId )
		{
			if( inId == 0 ) { return false; }
			return m_AssetList.Remove( inId );
		}


		public static bool RemoveAsset( string inPath )
		{
			if( String.IsNullOrWhiteSpace( inPath ) || inPath.Length < 4 ) { return false; }

			inPath = inPath.Replace( "\\\\", "/" ).Replace( "\\", "/" );
			if( inPath.StartsWith( "content/" ) || inPath.StartsWith( "Content/" ) )
			{
				inPath = inPath.Substring( 8 );
			}

			return RemoveAsset( Core.CalculateAssetIdentifier( inPath ) );
		}


		public static void PrintAssetList( string[] args )
		{
			Core.WriteLine( "---------------------------- Registered Asset List ----------------------------" );

			foreach( var entry in m_AssetList )
			{
				Core.WriteLine( "-> [", entry.Key, "] ", entry.Value );
			}

			Core.WriteLine( "---------------------------------------------------------------------------------------" );
		}


		public static void DeleteAsset( string[] args )
		{
			// Get and clean the target path
			string path = String.Concat( args ).Replace( "\\\\", "/" ).Replace( "\\", "/" );
			if( String.IsNullOrWhiteSpace( path ) || path.Length < 4 )
			{
				Core.WriteLine( "--> Failed to delete \"", path, "\" because the path is invalid" );
				return;
			}

			string cleanPath = path;
			if( cleanPath.StartsWith( "content/" ) || cleanPath.StartsWith( "Content/" ) )
			{
				cleanPath = cleanPath.Substring( 8 );
			}

			if( path.StartsWith( "Content/" ) )
			{
				path = "c" + path.Substring( 1 );
			}
			else if( !path.StartsWith( "content/" ) )
			{
				path = "content/" + path;
			}

			// Delete from the list
			bool bFound = false;
			foreach( var entry in m_AssetList )
			{
				if( entry.Value == cleanPath )
				{
					m_AssetList.Remove( entry.Key );
					bFound = true;
					break;
				}
			}

			// Delete the file if we can find it
			if( !File.Exists( path ) )
			{
				if( bFound )
				{
					Core.WriteLine( "[ERROR] AssetCache: Found asset \"", path, "\" in cache, but the file wasnt found!" );
					return;
				}
				else
				{
					Core.WriteLine( "--> Asset \"", path, "\" was not found!" );
				}
			}
			else
			{
				File.Delete( path );

				if( bFound )
				{
					Core.WriteLine( "--> Asset \"", path, "\" was deleted!" );
				}
				else
				{
					Core.WriteLine( "--> Asset \"", path, "\" was deleted from the filesystem, but an entry wasnt found in the cache" );
				}
			}
		}


		public static void RegisterConsoleCommands()
		{
			CommandInfo printAssetsCommand = new CommandInfo
			{
				Base = "assets_print",
				MinArgs = 0,
				MaxArgs = 0,
				Description = "Prints out the list of registered assets",
				Usage = "assets_print",
				Callback = PrintAssetList
			};

			CommandInfo deleteAssetCommand = new CommandInfo
			{
				Base = "asset_delete",
				MinArgs = 1,
				MaxArgs = -1,
				Description = "Deletes an asset from disk and the associated cache entry",
				Usage = "asset_delete [path]",
				Callback = DeleteAsset
			};

			Core.RegisterCommand( printAssetsCommand );
			Core.RegisterCommand( deleteAssetCommand );
		}
	}
}
