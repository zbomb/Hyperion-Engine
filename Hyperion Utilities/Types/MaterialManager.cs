using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;



namespace Hyperion
{
    static class MaterialManager
    {
        /*
         *  Static Members
        */
        private static MaterialEditor m_Form = null;


        /*-------------------------------------------------------------------------------------------------
         *      Console Commands
         * ------------------------------------------------------------------------------------------------*/
        public static void RegisterConsoleCommands()
        {
            var openCommand = new CommandInfo
            {
                Base        = "material_edit",
                Usage       = "material_edit [path]",
                Description = "Edit the target material, the path is relative to the content direcotry",
                MinArgs     = 1,
                MaxArgs     = 1,
                Callback    = (a) =>
                {
                    // We need to calculate the hash before calling the function to open the editor
                    uint id = Core.CalculateAssetIdentifier( a.First() );
                    if( id == 0 )
                    {
                        Core.WriteLine( "Failed to open editor, invalid material name \"", a.First(), "\"" );
                        return;
                    }

                    OpenEditor( a.First(), id );
                }
            };

            var closeCommand = new CommandInfo
            {
                Base        = "material_close",
                Usage       = "material_close",
                Description = "Closes the material editor if its open, saves any work before closing",
                MinArgs     = 0,
                MaxArgs     = 0,
                Callback    = (a) => CloseEditor()
            };

            var createCommand = new CommandInfo
            {
                Base = "material_create",
                Usage = "material_create [path]",
                Description = "Creates a new material using the specified path, the path is relative to the content direcotry",
                MinArgs = 1,
                MaxArgs = 1,
                Callback = (a) => CreateNew( a.First() )
            };

            Core.RegisterCommand( openCommand );
            Core.RegisterCommand( closeCommand );
            Core.RegisterCommand( createCommand );
        }

        public static void CloseEditor()
        {
            if( m_Form != null )
            {
                if( m_Form.IsDisposed )
                {
                    m_Form = null;
                }
                else
                {
                    m_Form.Close();
                    m_Form = null;
                }
            }
        }

        public static bool IsEditorOpen()
        {
            if( m_Form != null )
            {
                return !m_Form.IsDisposed;
            }
            else
            {
                return false;
            }
        }


        public static bool OpenEditor( string localPath, uint inId )
        {
            // The input path is rooted in the game directory
            // i.e. A valid path would be content/material/my_material.hmat
            // First, check if the editor is already open
            if( m_Form != null && !m_Form.IsDisposed )
            {
                Core.WriteLine( "Failed to open material editor.. the window is already open" );
                return false;
            }

            string relPath;
            if( !localPath.StartsWith( "content/" ) )
            {
                relPath = "content/" + localPath;
            }
            else { relPath = localPath; }

            if( localPath.Length < 6 ||
                !localPath.EndsWith( ".hmat" ) ||
                !File.Exists( relPath ) )
            {
                Core.WriteLine( "[Warning] MaterialEditor: Failed to open \"", localPath, "\" because the file doesnt exist/invlaid path" );
                return false;
            }

            // Now, we want to open the file, and leave it open while the editor is open
            try
            {
                m_Form = new MaterialEditor( relPath, inId );
                m_Form.ShowDialog( Program.GetWindow() );
                m_Form?.Dispose();
                m_Form = null;

                return true;
            }
            catch( Exception ex )
            {
                Core.WriteLine( "[Warning] MaterialEditor: Failed to edit \"", localPath, "\" (", ex.Message, ")" );
                return false;
            }
        }


        public static void CreateNew( string fileName )
        {
            // Input should look like
            // materials/my_mat.hmat
            // The path is relative to the content folder, and the extension is optional

            // Spit out a warning if this material is outside of the normal material folder
            if( !fileName.StartsWith( "materials/" ) )
            {
                Core.WriteLine( "[Warning] Entered filepath \"", fileName, "\" doesnt appear to be inside of the material folder!" );
            }

            // We need to generate the valid filename, it needs to end with '.hmat' (must be lowercase)
            if( fileName.ToLower().EndsWith( ".hmat" ) )
            {
                fileName = fileName.Substring( 0, fileName.Length - 5 ) + ".hmat";
            }
            else
            {
                fileName += ".hmat";
            }

            // Prepend the content folder too
            string relPath = "content/" + fileName;

            // Check if the path is a proper local path
            if( Path.IsPathRooted( relPath ) )
            {
                Core.WriteLine( "Entered file path needs to be local to the content directory! \"", fileName, "\"" );
                return;
            }

            if( File.Exists( relPath ) )
            {
                Core.WriteLine( "Failed to create new material! File already exists \"", relPath, "\" Please delete it using \"file_delete [path]\" before creating material again" );
                return;
            }

            // Finally, we want to check if the filename is actually valid
            try
            {
                Directory.CreateDirectory( Path.GetDirectoryName( relPath ) );
                var info = new FileInfo( relPath );
            }
            catch( Exception )
            {
                // The file path is invalid
                Core.WriteLine( "Entered file path doesnt appear valid! \"", fileName, "\"" );
                return;
            }

            // Now, we want to write an empty material file, and then open it with the editor
            try
            {
                using( FileStream newFile = File.Create( relPath ) )
                {
                    if( newFile == null ) { throw new Exception( "failed to open file" ); }

                    // Now, we need to write an empty material file
                    // First, we have to write out the validation bytes
                    byte[] headerData = { 0x1A, 0xA1, 0xFF, 0x28, 0x9D, 0xD9, 0x00, 0x04 };
                    newFile.Write( headerData, 0, 8 );

                    // Next, 2 bytes for the list length (which is zero), and then, 6 bytes of reserved data (also zero)
                    byte[] otherData = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                    newFile.Write( otherData, 0, 8 );
                }
            }
            catch( Exception ex )
            {
                Core.WriteLine( "Failed to create new material file \"", fileName, "\" (", ex.Message, ")" );
                return;
            }

            // Add an entry to the manifest for this new file
            uint id = Core.CalculateAssetIdentifier( fileName );

            if( id == 0 )
            {
                Core.WriteLine( "Failed to open new material file in editor, because an asset ID couldnt be generated! Please restart tool program to properly load this asset in" );
                return;
            }

            // Now, that the file was created and written, we can open the material editor like normal
            if( !OpenEditor( relPath, id ) )
            {
                Core.WriteLine( "[Warning] MateriallEditor: Created new material file, but failed to open it in the editor!" );
            }

        }


        /*-------------------------------------------------------------------------------------------------
         *      Loading/Saving
         * ------------------------------------------------------------------------------------------------*/

        public static bool LoadMaterial( string inPath, out Material outMat )
        {
            outMat = null;

            // First, we need to validate the path and ensure we can load it
            if( ( inPath?.Length ?? 0 ) == 0 )
            {
                Core.WriteLine( "[Warning] MaterialManager: Failed to load material because the target path was invalid" );
                return false;
            }

            string assetPath    = inPath;
            FileStream file     = null;

            // We want to ensure we have two different versions of the path, the one that includes 'content/' and one that doesnt
            if( inPath.StartsWith( "content/" ) )
            {
                assetPath = inPath.Remove( 0, 8 );
            }
            else
            {
                inPath = "content/" + inPath;
            }

            uint assetId = Core.CalculateAssetIdentifier( inPath );
            if( assetId == 0 )
            {
                Core.WriteLine( "[Warning] MaterialManager: Failed to load material \"", inPath, "\" because the id ended up invalid" );
                return false;
            }

            // Attempt to open the file
            if( inPath.EndsWith( ".hmat" ) )
            {
                try
                {
                    file = File.Open( inPath, FileMode.Open );
                    if( file == null ) throw new Exception( "File was null after opening" );
                }
                catch( Exception Ex )
                {
                    Core.WriteLine( "[Warning] MaterialManager: Failed to load material \"", inPath, "\" because the file couldnt be opened (", Ex.Message, ")" );
                    file?.Dispose();
                    return false;
                }
            }
            else
            {
                Core.WriteLine( "[Warning] MaterialManager: Failed to load material \"", inPath, "\" because the extension was not valid" );
                return false;
            }

            // Now that the file is open, lets start reading...
            var headerData = new byte[ 16 ];
            try
            {
                if( file.Length < 16 )
                {
                    throw new Exception( "Not enough data" );
                }
                if( file.Read( headerData, 0, 16 ) != 16 )
                {
                    throw new Exception( "Failed to read header data" );
                }

                // Validate header information
                byte[] validHeader = { 0x1A, 0xA1, 0xFF, 0x28, 0x9D, 0xD9, 0x00, 0x04 };
                for( int i = 0; i < validHeader.Length; i++ )
                {
                    if( validHeader[ i ] != headerData[ i ] )
                    {
                        throw new Exception( "Invalid header data" );
                    }
                }
            }
            catch( Exception Ex )
            {
                Core.WriteLine( "[Warning] MaterialManager: Failed to load material \"", inPath, "\" (", Ex.Message, ")" );
                file?.Dispose();
                return false;
            }

            // Now we need to read the property count in
            ushort propertyCount    = Serialization.GetUInt16( headerData, 8, false );
            var Output              = new Material( assetPath, assetId );

            // And now, lets read in the material properties
            for( ushort i = 0; i < propertyCount; i++ )
            {
                // Read the static data for this property in
                var staticData = new byte[ 8 ];
                try
                {
                    if( file.Read( staticData, 0, 8 ) != 8 )
                    {
                        throw new Exception( "Not enough data to read the property in" );
                    }
                }
                catch( Exception Ex )
                {
                    Core.WriteLine( "[Warning] MaterialManager: Failed to read material property for \"", inPath, "\" (", Ex.Message, ")" );
                    file?.Dispose();
                    return false;
                }

                // Deserialize all of the static data
                var keyLength       = Serialization.GetUInt16( staticData, 0, false );
                var valueTypeNum    = Serialization.GetUInt8( staticData, 2 );
                var valueLength     = Serialization.GetUInt16( staticData, 3 );

                // And lets ensure the values are valid
                if( !Enum.IsDefined( typeof( MaterialPropertyType ), (int) valueTypeNum ) )
                {
                    Core.WriteLine( "[Warning] MaterialManager: Failed to read property in \"", inPath, "\" because the value type was invalid (", valueTypeNum, ")" );
                    file?.Dispose();
                    return false;
                }

                var valueType = (MaterialPropertyType) valueTypeNum;

                if( file.Length - file.Position < keyLength + valueLength )
                {
                    Core.WriteLine( "[Warning] MaterialManager: Failed to read property in \"", inPath, "\" because we hit the end of the file while reading the property" );
                    file?.Dispose();
                    return false;
                }

                // Now, read the rest of the data in
                byte[] keyValueData = new byte[ keyLength + valueLength ];
                try
                {
                    if( file.Read( keyValueData, 0, keyValueData.Length ) != keyValueData.Length )
                    {
                        throw new Exception( "Failed to read property data" );
                    }
                }
                catch( Exception Ex )
                {
                    Core.WriteLine( "[Warning] MaterialManager: Failed to read property in \"", inPath, "\" ", Ex.Message );
                    file?.Dispose();
                    return false;
                }

                // Now finally, we can read in the key and value
                string key = Serialization.GetString( keyValueData, 0, keyLength, StringEncoding.UTF16_BE ).ToLower();
                if( key == null || key.Length == 0 )
                {
                    Core.WriteLine( "[Warning] Materialmanager: Failed to read proeprty in \"", inPath, "\" because the key was invalid" );
                    file?.Dispose();
                    return false;
                }

                // Check if this key already exists
                if( Output.Properties.ContainsKey( key ) )
                {
                    Core.WriteLine( "[Warning] MaterialManager: Multiple proeprties in \"", inPath, "\" with the same key \"", key, "\"" );
                    continue;
                }

                object castedValue = null;

                switch( valueType )
                {
                    case MaterialPropertyType.Boolean:
                        castedValue = (object) Serialization.GetBoolean( keyValueData, keyLength );
                        break;
                    case MaterialPropertyType.Int32:
                        castedValue = (object) Serialization.GetInt32( keyValueData, keyLength, false );
                        break;
                    case MaterialPropertyType.UInt32:
                        castedValue = (object) Serialization.GetUInt32( keyValueData, keyLength, false );
                        break;
                    case MaterialPropertyType.Float:
                        castedValue = (object) Serialization.GetFloat( keyValueData, keyLength, false );
                        break;
                    case MaterialPropertyType.String:
                        castedValue = (object) Serialization.GetString( keyValueData, keyLength, valueLength, StringEncoding.UTF16_BE );
                        if( castedValue == null )
                        {
                            Core.WriteLine( "[Warning] MaterialManager: Failed to read string property for \"", inPath, "\"" );
                        }

                        break;
                    case MaterialPropertyType.Texture:
                        // Read the identifier
                        var id = Serialization.GetUInt32( keyValueData, keyLength, false );

                        // Now, lookup the texture path
                        string path = null;
                        var dir = new DirectoryInfo( "content/textures" );

                        foreach( var f in dir.GetFiles( "*.htx", SearchOption.AllDirectories ) )
						{
                            string p = f.Name;
                            if( !p.StartsWith( "textures/" ) ) { p = "textures/" + p; }

                            if( Core.CalculateAssetIdentifier( p ) == id )
							{
                                path = p;
                                break;
							}
						}

                        if( path == null )
                        {
                            Core.WriteLine( "[Warning] MaterialManager: Read a texture property for \"", inPath, "\" but couldnt find the texture path from identifier (", id, ")" );
                        }
                        else
                        {
                            castedValue = (object) new TextureReference
                            {
                                Hash = id,
                                Path = path
                            };
                        }

                        break;
                    default:
                        break;
                }

                // Now, make the entry and continue
                if( castedValue != null )
                {
                    Output.Properties[ key ] = castedValue;
                }
            }

            // Return the new material
            outMat = Output;
            file?.Dispose();
            return true;
        }


        public static bool SaveMaterial( Material inMat )
        {
            // Check parameters
            if( ( inMat?.Path?.Length ?? 0 ) == 0 || !inMat.Path.EndsWith( ".hmat" ) )
            {
                Core.WriteLine( "[Warning] MaterialManager: Failed to save material, it (or the path) was invalid!" );
                return false;
            }

            // We want to create a file for this material, if it doesnt already exist, then we need to also make an 
            // entry into the manifest manager, and also keep track of any texture refs while writing the file
            var fileData    = new List< byte >{ 0x1A, 0xA1, 0xFF, 0x28, 0x9D, 0xD9, 0x00, 0x04 };
            var refList     = new List< uint >();

            if( inMat.Properties.Count > UInt16.MaxValue )
            {
                Core.WriteLine( "[ERROR] MaterialManager: Failed to write material \"", inMat.Path, "\" because there is too many properties!" );
                return false;
            }

            // Next, uint16 big-endian describing the number of entries in the material
            byte[] countData = Serialization.FromUInt16( (UInt16) inMat.Properties.Count, false );
            fileData.AddRange( countData );

            // Add an aditional 6-bytes of reserved bytes
            fileData.AddRange( new byte[] { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } );

            // Now, loop through and start serializing the properties
            foreach( var Entry in inMat.Properties )
            {
                byte[] keyData = Serialization.FromString( Entry.Key, StringEncoding.UTF16_BE );
                if( keyData.Length > UInt16.MaxValue )
                {
                    Core.WriteLine( "[Warning] MaterialManager: Failed to write material \"", inMat.Path, "\" because one of the keys exceed the max length" );
                    return false;
                }

                // Next, we need to determine the var type for this property
                var VarType         = Entry.Value.GetType();
                byte VarTypeNum     = 0;
                byte[] VarData      = null;

                if( VarType == typeof( bool ) )
                {
                    VarTypeNum = (byte) MaterialPropertyType.Boolean;
                    VarData = Serialization.FromBoolean( (bool) Entry.Value );
                }
                else if( VarType == typeof( int ) )
                {
                    VarTypeNum = (byte) MaterialPropertyType.Int32;
                    VarData = Serialization.FromInt32( (int) Entry.Value, false );
                }
                else if( VarType == typeof( uint ) )
                {
                    VarTypeNum = (byte) MaterialPropertyType.UInt32;
                    VarData = Serialization.FromUInt32( (uint) Entry.Value, false );
                }
                else if( VarType == typeof( float ) )
                {
                    VarTypeNum = (byte) MaterialPropertyType.Float;
                    VarData = Serialization.FromFloat( (float) Entry.Value, false );
                }
                else if( VarType == typeof( string ) )
                {
                    VarTypeNum = (byte) MaterialPropertyType.String;
                    VarData = Serialization.FromString( (string) Entry.Value, StringEncoding.UTF16_BE );

                    if( VarData.Length > UInt16.MaxValue )
                    {
                        Core.WriteLine( "[Warning] MaterialManager: Issue while saving material \"", inMat.Path, "\"! String property is too long!" );
                        return false;
                    }
                }
                else if( VarType == typeof( TextureReference ) )
                {
                    VarTypeNum = (byte) MaterialPropertyType.Texture;
                    var tex     = (TextureReference) Entry.Value;

                    if( !refList.Contains( tex.Hash ) )
                    {
                        refList.Add( tex.Hash );
                    }

                    VarData = Serialization.FromUInt32( tex.Hash, false );
                }
                else
                {
                    Core.WriteLine( "[ERROR] MaterialEditor: Failed to save material file! Invalid property type found" );
                    return false;
                }

                // Write the length of the key into the file
                fileData.AddRange( Serialization.FromUInt16( (ushort) keyData.Length, false ) );

                // Now, write out the var type
                fileData.Add( VarTypeNum );

                // Then, add the value length
                fileData.AddRange( Serialization.FromUInt16( (ushort) VarData.Length, false ) );

                // Three bytes of reserved data
                fileData.AddRange( new byte[] { 0x00, 0x00, 0x00 } );

                // Write key data, and then value data
                fileData.AddRange( keyData );
                fileData.AddRange( VarData );
            }

            // Now that we have the data built for the file, we need to actually create/open it, and then write the data
            string relPath;
            if( inMat.Path.StartsWith( "content/" ) )
            {
                relPath = inMat.Path;
            }
            else
            {
                relPath = "content/" + inMat.Path;
            }

            FileStream fStream  = null;
            bool bWasCreate     = false;

            try
            {
                if( File.Exists( relPath ) )
                {
                    fStream = File.Open( relPath, FileMode.Open );
                }
                else
                {
                    fStream     = File.Create( relPath );
                    bWasCreate  = true;
                }

                if( fStream == null )
                {
                    throw new Exception( "File was null?" );
                }
            }
            catch( Exception Ex )
            {
                Core.WriteLine( "[Warning] MaterialManager: Failed to save material file \"", inMat.Path, "\"! ", Ex.Message );
                fStream?.Dispose();
                return false;
            }

            try
            {
                fStream.SetLength( 0 );
                fStream.Flush();

                fStream.Seek( 0, SeekOrigin.Begin );

                fStream.Write( fileData.ToArray(), 0, fileData.Count );
            }
            catch( Exception Ex )
            {
                Core.WriteLine( "[ERROR] MaterialManager: Failed to save material file \"", inMat.Path, "\"! Error thrown: ", Ex.Message );
                fStream?.Dispose();
                return false;
            }

            fStream?.Dispose();
            AssetIdentifierCache.RegisterAsset( relPath );

            return true;
        }


        

    }
}
