using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;


namespace Hyperion
{
    class ReferenceManager
    {
        class ManagerInfo
        {
            public string Name;
            public string Extension;
            public Func< string, uint, uint, bool > OnRefChanged;
            public Func< string, uint, bool > OnRefDeleted;
        }

        /*------------------------------------------------------------------------------------------
         *      Member Vars
         *------------------------------------------------------------------------------------------*/
        private static Dictionary< uint, List< uint > > m_RefTable = new Dictionary<uint, List<uint>>();
        private static Dictionary< string, ManagerInfo > m_Managers = new Dictionary<string, ManagerInfo>();


        /*------------------------------------------------------------------------------------------
         *      Member Functions
         *------------------------------------------------------------------------------------------*/

        public static void RegisterConsoleCommands()
        {
            var printCommand = new CommandInfo
            {
                Base = "refs_print",
                Usage = "regs_print",
                Description = "Prints a list of all asset references",
                MinArgs = 0,
                MaxArgs = 0,
                Callback = (a) => PrintCommand()
            };

            Core.RegisterCommand( printCommand );
        }

        private static void PrintCommand()
        {
            Core.WriteLine();
            Core.WriteLine( "Asset Reference List:" );

            foreach( var Entry in m_RefTable )
            {
                Core.WriteLine( "\t", Entry.Key, ":" );
                foreach( var Ref in Entry.Value )
                {
                    Core.WriteLine( "\t\t", Ref );
                }
            }

            Core.WriteLine();
        }


        internal static Dictionary<uint, List<uint>> GetRefTable() => m_RefTable;


        public static bool RegisterAssetType( string typeName, string extStr, Func< string, uint, uint, bool > onRefChanged, Func< string, uint, bool > onRefDeleted )
        {
            // Validate Parameters
            if( ( typeName?.Length ?? 0 ) == 0 || ( extStr?.Length ?? 0 ) == 0 || onRefChanged == null || onRefDeleted == null )
            {
                Core.WriteLine( "[ERROR] ReferenceManager: Failed to register new asset type, parameters were invalid or null" );
                return false;
            }

            extStr = extStr.ToLower();

            // Ensure this doesnt already exist
            if( m_Managers.ContainsKey( extStr ) )
            {
                Core.WriteLine( "[ERROR] ReferenceManager: Failed to register asset type \"", typeName, "\" because the extension provided (", extStr, ") is already in use" );
                return false;
            }

            // Create the entry
            m_Managers[ extStr ] = new ManagerInfo
            {
                Extension = extStr,
                Name = typeName,
                OnRefChanged = onRefChanged,
                OnRefDeleted = onRefDeleted
            };

            return true;
        }


        public static List< uint > GetRefsFrom( uint inAsset )
        {
            return m_RefTable.TryGetValue( inAsset, out List< uint > Output ) ? Output : null;
        }


        public static void SetRefsFrom( uint inAsset, List< uint > inList )
        {
            if( inAsset != 0 )
            {
                m_RefTable[ inAsset ] = inList;
                FlushToDisk();
            }
        }


        public static List< uint > GetRefsTo( uint inAsset )
        {
            List< uint > Output = new List< uint >();
            foreach( var Entry in m_RefTable )
            {
                if( Entry.Value.Contains( inAsset ) )
                {
                    Output.Add( Entry.Key );
                }
            }

            return Output;
        }


        public static void OnAssetRenamed( uint oldId, string newPath )
        {
            // First, we want to find all assets that are using the old identifier
            var refList             = GetRefsTo( oldId );
            var manifestManager     = Core.GetManifestManager();

            if( manifestManager == null )
            {
                Core.WriteLine( "[ERROR] ReferenceManager: Failed to update asset id! Couldnt get the asset manifest!" );
                return;
            }

            // We need to update the manifest, and get the new ID and ensure its valid
            uint newId = 0;
            if( !manifestManager.UpdateEntry( oldId, newPath, out newId ) )
            {
                Core.WriteLine( "[Warning] ReferenceManager: Failed to update asset id! Manifest manager refused the update" );
                return;
            }

            foreach( var assetId in refList )
            {
                // We want to get the path for each asset
                var assetPath = manifestManager.GetAssetPath( assetId );
                if( assetPath == null || assetPath.Length == 0 )
                {
                    Core.WriteLine( "[Warning] ReferenceManager: Failed to update bad ref in asset, because the path for the asset couldnt be found" );
                    continue;
                }

                // Now, find what manager this asset belongs to by checking the file extension
                var assetExt = Path.GetExtension( assetPath );
                ManagerInfo targetManager = null;

                if( !m_Managers.TryGetValue( assetExt.ToLower(), out targetManager ) )
                {
                    targetManager = null;
                }

                if( targetManager == null || targetManager.OnRefChanged == null )
                {
                    Core.WriteLine( "[Warning] ReferenceManager: Failed to update a reference in \"", assetPath, "\" because a manager couldnt be found for this file type (Or didnt have the event bound)" );
                    continue;
                }

                // Now, call the function for the manager to handle updating the actual file itself
                if( !targetManager.OnRefChanged( assetPath, oldId, newId ) )
                {
                    Core.WriteLine( "[Warning] ReferenceManager: Failed to update a reference in \"", assetPath, "\" because the ", targetManager.Name, " manager wasnt able to update the reference" );
                }
            }

            m_RefTable[ newId ] = m_RefTable[ oldId ];
            m_RefTable.Remove( oldId );
            FlushToDisk();
        }


        public static void OnAssetRemoved( uint inId )
        {
            // First, we want to find all assets that are referencing this asset
            var refList             = GetRefsTo( inId );
            var manifestManager     = Core.GetManifestManager();

            if( manifestManager == null )
            {
                Core.WriteLine( "[ERROR] ReferenceManager: Failed to update asset id! Couldnt get the asset manifest!" );
                return;
            }

            manifestManager.DeleteEntry( inId );

            foreach( var assetId in refList )
            {
                // We want to get the path for each asset
                var assetPath = manifestManager.GetAssetPath( assetId );
                if( assetPath == null || assetPath.Length == 0 )
                {
                    Core.WriteLine( "[Warning] ReferenceManager: Failed to update bad ref in asset, because the path for the asset couldnt be found" );
                    continue;
                }

                // Now, find what manager this asset belongs to by checking the file extension
                var assetExt = Path.GetExtension( assetPath );
                ManagerInfo targetManager = null;

                if( !m_Managers.TryGetValue( assetExt.ToLower(), out targetManager ) )
                {
                    targetManager = null;
                }

                if( targetManager == null || targetManager.OnRefChanged == null )
                {
                    Core.WriteLine( "[Warning] ReferenceManager: Failed to update a reference in \"", assetPath, "\" because a manager couldnt be found for this file type (Or didnt have the event bound)" );
                    continue;
                }

                // Now, call the function for the manager to handle updating the actual file itself
                if( !targetManager.OnRefDeleted( assetPath, inId ) )
                {
                    Core.WriteLine( "[Warning] ReferenceManager: Failed to delete reference in \"", assetPath, "\" because the ", targetManager.Name, " manager wasnt able to delete the reference" );
                }
            }

            m_RefTable.Remove( inId );
            FlushToDisk();
        }


        public static void ReadFromDisk()
        {
            if( !File.Exists( "reflist.hrl" ) )
            {
                Core.WriteLine( "---> There is no reference list on disk!" );
                return;
            }

            // First, we need to load in the header
            // This is 12 bytes, 8 for the validation, 4 for list count
            try
            {
                using( FileStream fStream = File.Open( "reflist.hrl", FileMode.Open ) )
                {
                    if( fStream == null ) throw new Exception( "File was null" );

                    // Read and validate header
                    var headerData = new byte[ 12 ];
                    if( fStream.Length < 12 || fStream.Read( headerData, 0, 12 ) != 12 )
                    {
                        throw new Exception( "Not enough data" );
                    }

                    var correctHeader = new byte[]{ 0x1A, 0xA1, 0xFF, 0x28, 0x9D, 0xD9, 0x00, 0xFF };
                    for( int i = 0; i < correctHeader.Length; i++ )
                    {
                        if( headerData[ i ] != correctHeader[ i ] )
                        {
                            throw new Exception( "Invalid header" );
                        }
                    }

                    m_RefTable.Clear();

                    // Go through and read each entry
                    uint entryCount = Serialization.GetUInt32( headerData, 8, false );
                    for( int i = 0; i < entryCount; i++ )
                    {
                        var staticData = new byte[ 8 ];
                        if( fStream.Read( staticData, 0, 8 ) != 8 )
                        {
                            throw new Exception( "Failed to read entry static data" );
                        }

                        uint assetId    = Serialization.GetUInt32( staticData, 0, false );
                        uint refCount   = Serialization.GetUInt32( staticData, 4, false );

                        var refData = new byte[ refCount * 4 ];
                        if( fStream.Read( refData, 0, refData.Length ) != refData.Length )
                        {
                            throw new Exception( "Failed to read ID list" );
                        }

                        var refList = new List< uint >();
                        for( int j = 0; j < refCount; j++ )
                        {
                            uint newId = Serialization.GetUInt32( refData, j * 4, false );
                            if( !refList.Contains( newId ) )
                            {
                                refList.Add( newId );
                            }
                        }

                        m_RefTable.Add( assetId, refList );
                    }
                }
            }
            catch( Exception Ex )
            {
                Core.WriteLine( "[Warning] ReferenceManager: Failed to read ref list from file! ", Ex.Message );
                m_RefTable.Clear();
            }
        }


        public static void FlushToDisk()
        {
            // We need to overwrite the current file with the ref list
            var fileData = new List< byte >{ 0x1A, 0xA1, 0xFF, 0x28, 0x9D, 0xD9, 0x00, 0xFF };

            // This is how the file will be stored
            // [HEADER DATA][ENTRY COUNT]
            // Then, each entry will look like...
            // [ID][REF COUNT][REFS...]

            fileData.AddRange( Serialization.FromUInt32( (uint) m_RefTable.Count ) );

            foreach( var Entry in m_RefTable )
            {
                fileData.AddRange( Serialization.FromUInt32( Entry.Key ) );
                fileData.AddRange( Serialization.FromUInt32( (uint) Entry.Value.Count ) );

                foreach( var Ref in Entry.Value )
                {
                    fileData.AddRange( Serialization.FromUInt32( Ref ) );
                }
            }

            // Now, that the data is built, we need to actually commit it to the file
            try
            {
                using( FileStream fStream = File.Open( "reflist.hrl", FileMode.OpenOrCreate ) )
                {
                    if( fStream == null ) throw new Exception( "File was null" );

                    // Now that the file is open, we need to write to it
                    fStream.SetLength( 0 );
                    fStream.Flush();
                    fStream.Write( fileData.ToArray(), 0, fileData.Count );
                }
            }
            catch( Exception Ex )
            {
                Core.WriteLine( "[Warning] ReferenceManager: Failed to write ref list to file! ", Ex.Message );
            }
        }
    }
}
