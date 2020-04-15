using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;


namespace Hyperion
{
    class MaterialWriter
    {
        private static MaterialEditor m_Form = null;

        public static void RegisterConsoleCommands()
        {
            var openCommand = new CommandInfo
            {
                Base        = "material_edit",
                Usage       = "material_edit [path]",
                Description = "Edit the target material, the path is relative to the content direcotry",
                MinArgs     = 1,
                MaxArgs     = 1,
                Callback    = (a) => OpenEditor( a.First() )
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
            if ( m_Form != null )
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


        public static bool OpenEditor( string localPath )
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
                var newFile = File.Open( relPath, FileMode.Open );
                if( newFile == null ) { throw new Exception( "failed to open" ); }

                m_Form = new MaterialEditor( newFile );
                m_Form.Show( Program.GetWindow() );

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

            // Now, that the file was created and written, we can open the material editor like normal
            if( !OpenEditor( relPath ) )
            {
                Core.WriteLine( "[Warning] MateriallEditor: Created new material file, but failed to open it in the editor!" );
            }
            
        }



    }
}
