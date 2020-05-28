using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;



namespace Hyperion
{
    class ManifestManager
    {
        /*------------------------------------------------------------------------------------------
         *      Members
         *------------------------------------------------------------------------------------------*/
        private FileStream m_File = null;
        private bool m_bDirty = false;
        private Dictionary< UInt32, string > m_Manifest = new Dictionary< UInt32, string >();

        /*------------------------------------------------------------------------------------------
         *      Console Commands
         *------------------------------------------------------------------------------------------*/
        public static void RegisterConsoleCommands()
        {
            CommandInfo printCommand = new CommandInfo
            {
                Base = "manifest_print",
                Usage = "manifest_print",
                Description = "Prints out the full asset manifest for physical assets",
                MinArgs = 0,
                MaxArgs = 0,
                Callback = (a) => PrintManifestListCommand()
            };

            CommandInfo reloadCommand = new CommandInfo
            {
                Base = "manifest_reload",
                Usage = "manifest_reload",
                Description = "Reloads the manifest from file",
                MinArgs = 0,
                MaxArgs = 0,
                Callback = (a) => ReloadManifestCommand()
            };

            CommandInfo purgeCommand = new CommandInfo
            {
                Base = "manifest_purge",
                Usage = "manifest_purge",
                Description = "Resaves the manifest file",
                MinArgs = 0,
                MaxArgs = 0,
                Callback = (a) => PurgeManifestCommand()
            };

            Core.RegisterCommand( printCommand );
            Core.RegisterCommand( reloadCommand );
            Core.RegisterCommand( purgeCommand );
        }

        public static void PrintManifestListCommand()
        {
            Core.WriteLine();
            Core.WriteLine( "Asset Manifest: " );

            var manifest = Core.GetManifestManager();
            if( manifest != null )
            {
                foreach( var entry in manifest.GetManifest() )
                {
                    Core.WriteLine( "\t[", entry.Key, "] ", entry.Value );
                }
            }

            Core.WriteLine();
        }

        public static void ReloadManifestCommand()
        {
            Core.WriteLine( "ManifestManager: Reloading manifest file..." );
            Core.GetManifestManager()?.LoadManifest();
        }


        public static void PurgeManifestCommand()
		{
            Core.WriteLine( "ManifestManager: Purging manifest..." );
            Core.GetManifestManager()?.MarkDirty();
            Core.GetManifestManager()?.FlushFile();
		}


        /*------------------------------------------------------------------------------------------
         *      Constructor/Destructor
         *------------------------------------------------------------------------------------------*/
        public ManifestManager()
        {
            // First, we need to open the file
            if( File.Exists( "content/manifest.hht" ) )
            {
                try
                {
                    m_File = File.Open( "content/manifest.hht", FileMode.Open );

                }
                catch( Exception Ex )
                {
                    Core.WriteLine( "[ERROR] ManifestManager: Failed to open manifest file! ", Ex.Message );
                }
            }
            else
            {
                Core.WriteLine( "[Warning] ManifestManager: Manifest file not found! Creating one..." );

                try
                {
                    m_File = File.Create( "content/manifest.hht" );
                    if( m_File == null ) throw new Exception( "Failed to create file" );
                }
                catch( Exception Ex )
                {
                    Core.WriteLine( "[ERROR] ManifestManager: Failed to create new manifest file! ", Ex.Message );
                }
            }

            // Now, load entries from the file
            LoadManifest();
        }

        ~ManifestManager()
        {
            // We want to ensure the file gets flushed and closed properly
            if( m_File != null )
            {
                m_File.Dispose();
                m_File = null;
            }
        }

        /*------------------------------------------------------------------------------------------
         *      Member Functions
         *------------------------------------------------------------------------------------------*/

        internal Dictionary<UInt32, string> GetManifest() => m_Manifest;


        private void LoadManifest()
        {
            // We want to clear the current list before reloading
            m_Manifest.Clear();

            if( m_File != null )
            {
                bool bValid = true;
                byte[] correctData = { 0x1A, 0xA1, 0xFF, 0x28, 0x9D, 0xD9, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 };

                if( m_File.Length >= 12 )
                {
                    // We want to read in header data and check if it is valid
                    byte[] headerData = new byte[ 12 ];
                    try
                    {
                        m_File.Seek( 0, SeekOrigin.Begin );
                        m_File.Read( headerData, 0, 12 );

                    }
                    catch( Exception Ex )
                    {
                        Core.WriteLine( "[ERROR] ManifestManager: Failed to read data! ", Ex.Message );
                        bValid = false;
                    }

                    if( bValid )
                    {
                        for( int i = 0; i < 8; i++ )
                        {
                            if( correctData[ i ] != headerData[ i ] )
                            {
                                bValid = false;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    bValid = false;
                }

                // If the current manifest is invalid, were going to recreate it from scratch
                // First, lets just write a valid header out to the file
                if( !bValid )
                {
                    Core.WriteLine( "[ERROR] ManifestManager: Current manifest file isnt valid! Rebuilding..." );

                    // Clear the file out
                    m_File.SetLength( 0 );
                    m_File.Flush( true );

                    // Recreate file
                    m_File.Dispose();
                    m_File = null;

                    try
                    {
                        m_File.Seek( 0, SeekOrigin.Begin );
                        m_File.Write( correctData, 0, 12 );

                    }
                    catch( Exception Ex )
                    {
                        Core.WriteLine( "[ERROR] ManifestManager: Failed to write data! ", Ex.Message );
                    }
                }
                else
                {
                    // We want to read in all existing values
                    m_File.Seek( 12, SeekOrigin.Begin );

                    while( m_File.Length - m_File.Position >= 6 )
                    {
                        byte[] staticData = new byte[ 6 ];
                        try
                        {
                            if( m_File.Read( staticData, 0, 6 ) != 6 ) { throw new Exception( "Hit end of stream!" ); }
                        }
                        catch( Exception Ex )
                        {
                            Core.WriteLine( "[ERROR] ManifestManager: Failed to read entry in manifest file! ", Ex.Message );
                            break;
                        }

                        // First, read in the hash code, UInt32 big endian number
                        var hashCode = Serialization.GetUInt32( staticData, 0, false );

                        // Next, read in the string length (in bytes) as a uint16 big endian number
                        var strLen = Serialization.GetUInt16( staticData, 4, false );

                        // Now read in the string data
                        if( m_File.Length - m_File.Position < strLen )
                        {
                            Core.WriteLine( "[ERROR] ManifestManager: Failed to read asset path from the manifest.. not enough data for the string!" );
                            break;
                        }

                        byte[] strData = new byte[ strLen ];
                        try
                        {
                            if( m_File.Read( strData, 0, strLen ) != strLen ) { throw new Exception( "Hit end of stream" ); }
                        }
                        catch( Exception Ex )
                        {
                            Core.WriteLine( "[ERROR] ManifestManager: Failed to read asset path from manifest! ", Ex.Message );
                            break;
                        }

                        // Read the asset path as a utf-8 string
                        string assetPath = Serialization.GetString( strData, 0, strData.Length, StringEncoding.UTF8 );
                        if( ( assetPath?.Length ?? 0 ) == 0 )
                        {
                            Core.WriteLine( "[Warning] ManifestManager: Invalid entry in manifest file! (", hashCode, ")" );
                            continue;
                        }

                        // Now we can add the entry into the manifest structure in memory
                        if( m_Manifest.ContainsKey( hashCode ) )
                        {
                            Core.WriteLine( "[Warning] ManifestManager: Collision detected in the manifest! Key: ", hashCode, ". Ignoring: ", assetPath );
                            continue;
                        }

                        // Check if this file actually exists
                        if( !File.Exists( "content/" + assetPath ) )
                        {
                            Core.WriteLine( "[ERROR] ManifestManager: There is an asset in the manifest, that no longer exists!" );

                            // TODO: Prompt the user, to allow them to select a replacment asset, and replace all references in the engine to this new asset

                            continue;
                        }

                        m_Manifest[ hashCode ] = assetPath;
                    }

                    Core.WriteLine( "---> ManifestManager: Loaded ", m_Manifest.Count, " entries from the asset manifest" );
                }

                // Perform asset seek to ensure the manifest is up to date
                ScanAssets();
            }
        }

        private bool IsAssetFile( string inStr )
        {
            return !inStr.Equals( "manifest.hht" );
        }

        public bool AddEntry( uint inId, string inPath )
        {
            // Validate the parameters
            if( inId == 0 || ( inPath?.Length ?? 0 ) == 0 )
            {
                Core.WriteLine( "[Warning] ManifestManager: Failed to add entry to manifest, invalid parameters" );
                return false;
            }

            // Check if this id already exists
            if( m_Manifest.ContainsKey( inId ) )
            {
                Core.WriteLine( "[Warning] ManifestManager: Failed to add entry to the manifest, because there was a collision with ID: ", inId );
                return false;
            }

            m_bDirty = true;
            m_Manifest.Add( inId, inPath );
            FlushFile();

            return true;
        }


        public bool DeleteEntry( uint inId )
        {
            if( inId == 0 )
            {
                Core.WriteLine( "[Warning] ManifestManager: Failed to delete entry from manifest, invalid parameters" );
                return false;
            }

            if( !m_Manifest.Remove( inId ) )
            {
                return false;
            }

            m_bDirty = true;
            FlushFile();

            return true;
        }


        public bool UpdateEntry( uint oldId, string newPath, out uint newId )
        {
            newId = 0;

            if( oldId == 0 || ( ( newPath?.Length ) == 0 ) )
            {
                Core.WriteLine( "[Warning] ManifestManager: Failed to update entry for ", oldId, " because either the id or the path is invalid" );
                return false;
            }

            var lowerPath = newPath.ToLower().Replace( "\\\\", "/" ).Replace( "\\", "/" );
            byte[] strData = Encoding.UTF8.GetBytes( lowerPath );
            newId = Hashing.ELFHash( strData );

            if( newId == 0 )
            {
                Core.WriteLine( "[Warning] ManifestManager: failed to update entry for ", oldId, " because the new path (", newPath, ") generated an invalid id!" );
                return false;
            }

            if( m_Manifest.ContainsKey( newId ) )
            {
                Core.WriteLine( "[Warning] ManifestManager: Failed to update entry for ", oldId, " because the new path (", newPath, ") generated a collision (", newId, ")" );
                return false;
            }

            m_bDirty = true;
            m_Manifest.Remove( oldId );
            m_Manifest.Add( newId, lowerPath );

            FlushFile();

            return true;
        }

        public uint CalculateAssetId( string inPath )
        {
            if( ( inPath?.Length ?? 0 ) == 0 )
            {
                return 0;
            }

            var lowerPath = inPath.ToLower().Replace( "\\\\", "/" ).Replace( "\\", "/" );
            byte[] strData = Encoding.UTF8.GetBytes( lowerPath );
            return Hashing.ELFHash( strData );
        }

        public void ScanAssets()
        {
            // We want to look for any assets in the content folder, and generate manifest entries, and then update the manifest file
            if( m_File != null )
            {
                Core.WriteLine( "---> Performing asset scan..." );

                var assetFiles = Directory.GetFiles( "content", "*.h*", SearchOption.AllDirectories ).ToArray();
                uint assetCounter = 0;

                foreach( var f in assetFiles )
                {
                    string assetPath = f.Substring( 8, f.Length - 8 ); // Take the 'content/' part of the path off
                    assetPath = assetPath.Replace( "\\\\", "/" ).Replace( "\\", "/" );
                    if( IsAssetFile( assetPath ) )
                    {
                        // We have two options, either lookup the value, or perform the hash and lookup the key
                        byte[] strData = Encoding.UTF8.GetBytes( assetPath );
                        uint hashCode = Hashing.ELFHash( strData );

                        if( !m_Manifest.ContainsKey( hashCode ) )
                        {
                            Core.WriteLine( "---> Found new asset: ", assetPath );

                            m_bDirty = true;
                            m_Manifest[ hashCode ] = assetPath;
                            assetCounter++;
                        }
                    }
                }

                Core.WriteLine( "---> Found ", assetCounter, " new assets!" );
                FlushFile();
            }
        }


        public void MarkDirty()
		{
            m_bDirty = true;
		}

        public void FlushFile()
        {
            if( m_File != null && m_bDirty )
            {
                // We want to build the byte buffer and perform a single file write, start out with the static header data
                List< byte > fileData = new List< byte >
                { 
                    0x1A, 0xA1, 0xFF, 0x28, 0x9D, 0xD9, 
                    0x00, 0x01, 0x00, 0x00, 0x00, 0x00 
                };

                // Now we need to loop through each entry and write them to the byte list
                foreach( var entry in m_Manifest )
                {
                    byte[] strData = Serialization.FromString( entry.Value, StringEncoding.UTF8 );
                    if( strData.Length > UInt16.MaxValue )
                    {
                        Core.WriteLine( "[Warning] ManifestManager: Failed to write entry to file.. string length exceeds max! Identifier: ", entry.Key );
                        continue;
                    }

                    fileData.AddRange( Serialization.FromUInt32( entry.Key, false ) );
                    fileData.AddRange( Serialization.FromUInt16( (UInt16)strData.Length, false ) );
                    fileData.AddRange( strData );
                }

                // Now, we need to actually perform the writing of the file, first, clear it
                try
                {
                    m_File.SetLength( 0 );
                    m_File.Flush();

                    m_File.Write( fileData.ToArray(), 0, fileData.Count );
                    m_File.Flush();

                    Core.WriteLine( "ManifestManager: Wrote manifest to file! ", m_Manifest.Count, " entries" );
                    m_bDirty = false;
                }
                catch( Exception Ex )
                {
                    Core.WriteLine( "[Warning] ManifestManager: Failed to write manifest file! ", Ex.Message );
                }
            }
        }

        public string GetAssetPath( uint inId )
        {
            string outStr;
            m_Manifest.TryGetValue( inId, out outStr );

            return outStr;

        }

    }
}
